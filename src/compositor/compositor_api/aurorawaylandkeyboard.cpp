// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandkeyboard.h"
#include "aurorawaylandkeyboard_p.h"
#include <LiriAuroraCompositor/WaylandKeymap>
#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandSeat>
#include <LiriAuroraCompositor/WaylandClient>

#include <QtCore/QFile>
#include <QtCore/QStandardPaths>

#include <QKeyEvent>
#include <fcntl.h>
#include <unistd.h>
#if LIRI_FEATURE_aurora_xkbcommon
#include <sys/mman.h>
#include <sys/types.h>
#include <xkbcommon/xkbcommon-names.h>
#endif

namespace Aurora {

namespace Compositor {

WaylandKeyboardPrivate::WaylandKeyboardPrivate(WaylandSeat *seat)
    : seat(seat)
{
}

WaylandKeyboardPrivate::~WaylandKeyboardPrivate()
{
#if LIRI_FEATURE_aurora_xkbcommon
    if (xkbContext()) {
        if (keymap_area)
            munmap(keymap_area, keymap_size);
        if (keymap_fd >= 0)
            close(keymap_fd);
    }
#endif
}

WaylandKeyboardPrivate *WaylandKeyboardPrivate::get(WaylandKeyboard *keyboard)
{
    return keyboard->d_func();
}

void WaylandKeyboardPrivate::checkFocusResource(Resource *keyboardResource)
{
    if (!keyboardResource || !focus)
        return;

    // this is already the current  resource, do no send enter twice
    if (focusResource == keyboardResource)
        return;

    // check if new wl_keyboard resource is from the client owning the focus surface
    if (wl_resource_get_client(focus->resource()) == keyboardResource->client()) {
        sendEnter(focus, keyboardResource);
        focusResource = keyboardResource;
    }
}

void WaylandKeyboardPrivate::sendEnter(WaylandSurface *surface, Resource *keyboardResource)
{
    uint32_t serial = compositor()->nextSerial();
    send_modifiers(keyboardResource->handle, serial, modsDepressed, modsLatched, modsLocked, group);
    send_enter(keyboardResource->handle, serial, surface->resource(), QByteArray::fromRawData((char *)keys.data(), keys.size() * sizeof(uint32_t)));
}

void WaylandKeyboardPrivate::focused(WaylandSurface *surface)
{
    if (!seat->isInputAllowed(surface))
        return;

    if (surface && surface->isCursorSurface())
        surface = nullptr;
    if (focus != surface) {
        if (focusResource) {
            uint32_t serial = compositor()->nextSerial();
            send_leave(focusResource->handle, serial, focus->resource());
        }
        focusDestroyListener.reset();
        if (surface)
            focusDestroyListener.listenForDestruction(surface->resource());
    }

    Resource *resource = surface ? resourceMap().value(surface->waylandClient()) : 0;

    if (resource && (focus != surface || focusResource != resource))
        sendEnter(surface, resource);

    focusResource = resource;
    focus = surface;
    Q_EMIT q_func()->focusChanged(focus);
}


void WaylandKeyboardPrivate::keyboard_bind_resource(wl_keyboard::Resource *resource)
{
    // Send repeat information
    if (resource->version() >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
        send_repeat_info(resource->handle, repeatRate, repeatDelay);

#if LIRI_FEATURE_aurora_xkbcommon
    if (xkbContext()) {
        send_keymap(resource->handle, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1,
                    keymap_fd, keymap_size);
    } else
#endif
    {
        int null_fd = open("/dev/null", O_RDONLY);
        send_keymap(resource->handle, WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP,
                    null_fd, 0);
        close(null_fd);
    }
    checkFocusResource(resource);
}

void WaylandKeyboardPrivate::keyboard_destroy_resource(wl_keyboard::Resource *resource)
{
    if (focusResource == resource)
        focusResource = nullptr;
}

void WaylandKeyboardPrivate::keyboard_release(wl_keyboard::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void WaylandKeyboardPrivate::keyEvent(uint code, uint32_t state)
{
    uint key = toWaylandKey(code);

    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        keys << key;
    } else {
        keys.removeAll(key);
    }
}

void WaylandKeyboardPrivate::sendKeyEvent(uint code, uint32_t state)
{
    uint32_t time = compositor()->currentTimeMsecs();
    uint32_t serial = compositor()->nextSerial();
    uint key = toWaylandKey(code);
    if (focusResource)
        send_key(focusResource->handle, serial, time, key, state);
}

#if LIRI_FEATURE_aurora_xkbcommon
void WaylandKeyboardPrivate::maybeUpdateXkbScanCodeTable()
{
    if (!scanCodesByQtKey.isEmpty() || !xkbState())
        return;

    if (xkb_keymap *keymap = xkb_state_get_keymap(xkbState())) {
        xkb_keymap_key_for_each(keymap, [](xkb_keymap *keymap, xkb_keycode_t keycode, void *d){
            auto *scanCodesByQtKey = static_cast<QMap<ScanCodeKey, uint>*>(d);
            uint numLayouts = xkb_keymap_num_layouts_for_key(keymap, keycode);
            for (uint layout = 0; layout < numLayouts; ++layout) {
                const xkb_keysym_t *syms = nullptr;
                xkb_keymap_key_get_syms_by_level(keymap, keycode, layout, 0, &syms);
                if (!syms)
                    continue;

                Qt::KeyboardModifiers mods = {};
                int qtKey = XkbCommon::keysymToQtKey(syms[0], mods, nullptr, 0, false, false);
                if (qtKey != 0)
                    scanCodesByQtKey->insert({layout, qtKey}, keycode);
            }
        }, &scanCodesByQtKey);

        shiftIndex = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_SHIFT);
        controlIndex = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_CTRL);
        altIndex = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_ALT);
    }
}

void WaylandKeyboardPrivate::resetKeyboardState()
{
    if (!xkbContext())
        return;

    while (!keys.isEmpty()) {
        uint32_t code = fromWaylandKey(keys.first());
        keyEvent(code, WL_KEYBOARD_KEY_STATE_RELEASED);
        updateModifierState(code, WL_KEYBOARD_KEY_STATE_RELEASED);
    }
}
#endif

void WaylandKeyboardPrivate::updateModifierState(uint code, uint32_t state)
{
#if LIRI_FEATURE_aurora_xkbcommon
    if (!xkbContext())
        return;

    xkb_state_update_key(xkbState(), code, state == WL_KEYBOARD_KEY_STATE_PRESSED ? XKB_KEY_DOWN : XKB_KEY_UP);

    uint32_t modsDepressed = xkb_state_serialize_mods(xkbState(), XKB_STATE_MODS_DEPRESSED);
    uint32_t modsLatched = xkb_state_serialize_mods(xkbState(), XKB_STATE_MODS_LATCHED);
    uint32_t modsLocked = xkb_state_serialize_mods(xkbState(), XKB_STATE_MODS_LOCKED);
    uint32_t group = xkb_state_serialize_layout(xkbState(), XKB_STATE_LAYOUT_EFFECTIVE);

    if (this->modsDepressed == modsDepressed
            && this->modsLatched == modsLatched
            && this->modsLocked == modsLocked
            && this->group == group)
        return;

    this->modsDepressed = modsDepressed;
    this->modsLatched = modsLatched;
    this->modsLocked = modsLocked;
    this->group = group;

    if (focusResource) {
        send_modifiers(focusResource->handle, compositor()->nextSerial(), modsDepressed,
                       modsLatched, modsLocked, group);

        Qt::KeyboardModifiers currentState = Qt::NoModifier;
        if (xkb_state_mod_index_is_active(xkbState(), shiftIndex, XKB_STATE_MODS_EFFECTIVE) == 1)
            currentState |= Qt::ShiftModifier;
        if (xkb_state_mod_index_is_active(xkbState(), controlIndex, XKB_STATE_MODS_EFFECTIVE) == 1)
            currentState |= Qt::ControlModifier;
        if (xkb_state_mod_index_is_active(xkbState(), altIndex, XKB_STATE_MODS_EFFECTIVE) == 1)
            currentState |= Qt::AltModifier;
        currentModifierState = currentState;
    }
#else
    Q_UNUSED(code);
    Q_UNUSED(state);
#endif
}

// If there is no key currently pressed, update the keymap right away.
// Otherwise, delay the update when keys are released
// see http://lists.freedesktop.org/archives/wayland-devel/2013-October/011395.html
void WaylandKeyboardPrivate::maybeUpdateKeymap()
{
    // There must be no keys pressed when changing the keymap,
    // see http://lists.freedesktop.org/archives/wayland-devel/2013-October/011395.html
    if (!pendingKeymap || !keys.isEmpty())
        return;

    pendingKeymap = false;
#if LIRI_FEATURE_aurora_xkbcommon
    if (!xkbContext())
        return;

    createXKBKeymap();
    const auto resMap = resourceMap();
    for (Resource *res : resMap) {
        send_keymap(res->handle, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, keymap_fd, keymap_size);
    }

    xkb_state_update_mask(xkbState(), 0, modsLatched, modsLocked, 0, 0, 0);
    if (focusResource)
        send_modifiers(focusResource->handle,
                       compositor()->nextSerial(),
                       modsDepressed,
                       modsLatched,
                       modsLocked,
                       group);
#endif
}

// In all current XKB keymaps there's a constant offset of 8 (for historical
// reasons) from hardware/evdev scancodes to XKB keycodes. On X11, we pass
// XKB keycodes (as sent by X server) via QKeyEvent::nativeScanCode. eglfs+evdev
// adds 8 for consistency, see qtbase/05c07c7636012ebb4131ca099ca4ea093af76410.
// eglfs+libinput also adds 8, for the same reason. Wayland protocol uses
// hardware/evdev scancodes, thus we need to subtract 8 before sending the event
// out and add it when mapping back.
#define QTWAYLANDKEYBOARD_XKB_HISTORICAL_OFFSET 8

uint WaylandKeyboardPrivate::fromWaylandKey(const uint key)
{
#if LIRI_FEATURE_aurora_xkbcommon
    const uint offset = QTWAYLANDKEYBOARD_XKB_HISTORICAL_OFFSET;
    return key + offset;
#else
    return key;
#endif
}

uint WaylandKeyboardPrivate::toWaylandKey(const uint nativeScanCode)
{
#if LIRI_FEATURE_aurora_xkbcommon
    const uint offset = QTWAYLANDKEYBOARD_XKB_HISTORICAL_OFFSET;
    Q_ASSERT(nativeScanCode >= offset);
    return nativeScanCode - offset;
#else
    return nativeScanCode;
#endif
}

#if LIRI_FEATURE_aurora_xkbcommon
static int createAnonymousFile(size_t size)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (path.isEmpty())
        return -1;

    QByteArray name = QFile::encodeName(path + QStringLiteral("/qtwayland-XXXXXX"));

    int fd = mkstemp(name.data());
    if (fd < 0)
        return -1;

    long flags = fcntl(fd, F_GETFD);
    if (flags == -1 || fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
        close(fd);
        fd = -1;
    }
    unlink(name.constData());

    if (fd < 0)
        return -1;

    if (ftruncate(fd, size) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

void WaylandKeyboardPrivate::createXKBState(xkb_keymap *keymap)
{
    char *keymap_str = xkb_keymap_get_as_string(keymap, XKB_KEYMAP_FORMAT_TEXT_V1);
    if (!keymap_str) {
        qWarning("Failed to compile global XKB keymap");
        return;
    }

    keymap_size = strlen(keymap_str) + 1;
    if (keymap_fd >= 0)
        close(keymap_fd);
    keymap_fd = createAnonymousFile(keymap_size);
    if (keymap_fd < 0) {
        qWarning("Failed to create anonymous file of size %lu", static_cast<unsigned long>(keymap_size));
        return;
    }

    keymap_area = static_cast<char *>(mmap(nullptr, keymap_size, PROT_READ | PROT_WRITE, MAP_SHARED, keymap_fd, 0));
    if (keymap_area == MAP_FAILED) {
        close(keymap_fd);
        keymap_fd = -1;
        qWarning("Failed to map shared memory segment");
        return;
    }

    strcpy(keymap_area, keymap_str);
    free(keymap_str);

    mXkbState.reset(xkb_state_new(keymap));
    if (!mXkbState)
        qWarning("Failed to create XKB state");
}

void WaylandKeyboardPrivate::createXKBKeymap()
{
    if (!xkbContext())
        return;

    WaylandKeymap *keymap = seat->keymap();
    QByteArray rules = keymap->rules().toLocal8Bit();
    QByteArray model = keymap->model().toLocal8Bit();
    QByteArray layout = keymap->layout().toLocal8Bit();
    QByteArray variant = keymap->variant().toLocal8Bit();
    QByteArray options = keymap->options().toLocal8Bit();

    if (!layout.isEmpty() && !layout.contains("us")) {
        // This is needed for shortucts like "ctrl+c" to function even when
        // user has selected only non-latin keyboard layouts, e.g. 'ru'.
        layout.append(",us");
        variant.append(",");
    }

    struct xkb_rule_names rule_names = {
        rules.constData(),
        model.constData(),
        layout.constData(),
        variant.constData(),
        options.constData()
    };

    XkbCommon::ScopedXKBKeymap xkbKeymap(xkb_keymap_new_from_names(xkbContext(), &rule_names,
                                                                    XKB_KEYMAP_COMPILE_NO_FLAGS));
    if (xkbKeymap) {
        scanCodesByQtKey.clear();
        createXKBState(xkbKeymap.get());
    } else {
        qWarning("Failed to load the '%s' XKB keymap.", qPrintable(keymap->layout()));
    }
}
#endif // LIRI_FEATURE_aurora_xkbcommon

void WaylandKeyboardPrivate::sendRepeatInfo()
{
    const auto resMap = resourceMap();
    for (Resource *resource : resMap) {
        if (resource->version() >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
            send_repeat_info(resource->handle, repeatRate, repeatDelay);
    }
}

/*!
 * \class WaylandKeyboard
 * \inmodule AuroraCompositor
 * \since 5.8
 * \brief The WaylandKeyboard class represents a keyboard device.
 *
 * This class provides access to the keyboard device in a WaylandSeat. It corresponds to
 * the Wayland interface wl_keyboard.
 */

/*!
 * Constructs a WaylandKeyboard for the given \a seat and with the given \a parent.
 */
WaylandKeyboard::WaylandKeyboard(WaylandSeat *seat, QObject *parent)
    : WaylandObject(* new WaylandKeyboardPrivate(seat), parent)
{
    Q_D(WaylandKeyboard);
    connect(&d->focusDestroyListener, &WaylandDestroyListener::fired, this, &WaylandKeyboard::focusDestroyed);
    auto keymap = seat->keymap();
    connect(keymap, &WaylandKeymap::layoutChanged, this, &WaylandKeyboard::updateKeymap);
    connect(keymap, &WaylandKeymap::variantChanged, this, &WaylandKeyboard::updateKeymap);
    connect(keymap, &WaylandKeymap::optionsChanged, this, &WaylandKeyboard::updateKeymap);
    connect(keymap, &WaylandKeymap::rulesChanged, this, &WaylandKeyboard::updateKeymap);
    connect(keymap, &WaylandKeymap::modelChanged, this, &WaylandKeyboard::updateKeymap);
#if LIRI_FEATURE_aurora_xkbcommon
    d->createXKBKeymap();
#endif
}

/*!
 * Returns the seat for this WaylandKeyboard.
 */
WaylandSeat *WaylandKeyboard::seat() const
{
    Q_D(const WaylandKeyboard);
    return d->seat;
}

/*!
 * Returns the compositor for this WaylandKeyboard.
 */
WaylandCompositor *WaylandKeyboard::compositor() const
{
    Q_D(const WaylandKeyboard);
    return d->seat->compositor();
}

/*!
 * \internal
 */
void WaylandKeyboard::focusDestroyed(void *data)
{
    Q_UNUSED(data);
    Q_D(WaylandKeyboard);
    d->focusDestroyListener.reset();

    d->focus = nullptr;
    d->focusResource = nullptr;
}

void WaylandKeyboard::updateKeymap()
{
    Q_D(WaylandKeyboard);
    d->pendingKeymap = true;
    d->maybeUpdateKeymap();
}

/*!
 * Returns the client that currently has keyboard focus.
 */
WaylandClient *WaylandKeyboard::focusClient() const
{
    Q_D(const WaylandKeyboard);
    if (!d->focusResource)
        return nullptr;
    return WaylandClient::fromWlClient(compositor(), d->focusResource->client());
}

/*!
 * Sends the current key modifiers to \a client with the given \a serial.
 */
void WaylandKeyboard::sendKeyModifiers(WaylandClient *client, uint32_t serial)
{
    Q_D(WaylandKeyboard);
    PrivateServer::wl_keyboard::Resource *resource = d->resourceMap().value(client->client());
    if (resource)
        d->send_modifiers(resource->handle, serial, d->modsDepressed, d->modsLatched, d->modsLocked, d->group);
}

/*!
 * Sends a key press event with the key \a code to the current keyboard focus.
 */
void WaylandKeyboard::sendKeyPressEvent(uint code)
{
    Q_D(WaylandKeyboard);
    d->sendKeyEvent(code, WL_KEYBOARD_KEY_STATE_PRESSED);
}

/*!
 * Sends a key release event with the key \a code to the current keyboard focus.
 */
void WaylandKeyboard::sendKeyReleaseEvent(uint code)
{
    Q_D(WaylandKeyboard);
    d->sendKeyEvent(code, WL_KEYBOARD_KEY_STATE_RELEASED);
}

void WaylandKeyboardPrivate::checkAndRepairModifierState(QKeyEvent *ke)
{
#if LIRI_FEATURE_aurora_xkbcommon
    if (ke->modifiers() != currentModifierState) {
        if (focusResource && ke->key() != Qt::Key_Shift
                && ke->key() != Qt::Key_Control && ke->key() != Qt::Key_Alt) {
            // Only repair the state for non-modifier keys
            // ### slightly awkward because the standard modifier handling
            // is done by Internal::WindowSystemEventHandler after the
            // key event is delivered
            uint32_t mods = 0;

            if (shiftIndex == 0 && controlIndex == 0)
                maybeUpdateXkbScanCodeTable();

            if (ke->modifiers() & Qt::ShiftModifier)
                mods |= 1 << shiftIndex;
            if (ke->modifiers() & Qt::ControlModifier)
                mods |= 1 << controlIndex;
            if (ke->modifiers() & Qt::AltModifier)
                mods |= 1 << altIndex;
            qCDebug(gLcAuroraCompositor) << "Keyboard modifier state mismatch detected for event" << ke << "state:" << currentModifierState << "repaired:" << Qt::hex << mods;
            send_modifiers(focusResource->handle, compositor()->nextSerial(), mods,
                    0, 0, group);
            currentModifierState = ke->modifiers();
        }
    }
#else
    Q_UNUSED(ke);
#endif
}

/*!
 * Returns the current repeat rate.
 */
quint32 WaylandKeyboard::repeatRate() const
{
    Q_D(const WaylandKeyboard);
    return d->repeatRate;
}

/*!
 * Sets the repeat rate to \a rate.
 */
void WaylandKeyboard::setRepeatRate(quint32 rate)
{
    Q_D(WaylandKeyboard);

    if (d->repeatRate == rate)
        return;

    d->sendRepeatInfo();

    d->repeatRate = rate;
    Q_EMIT repeatRateChanged(rate);
}

/*!
 * Returns the current repeat delay.
 */
quint32 WaylandKeyboard::repeatDelay() const
{
    Q_D(const WaylandKeyboard);
    return d->repeatDelay;
}

/*!
 * Sets the repeat delay to \a delay.
 */
void WaylandKeyboard::setRepeatDelay(quint32 delay)
{
    Q_D(WaylandKeyboard);

    if (d->repeatDelay == delay)
        return;

    d->sendRepeatInfo();

    d->repeatDelay = delay;
    Q_EMIT repeatDelayChanged(delay);
}

/*!
 * Returns the currently focused surface.
 */
WaylandSurface *WaylandKeyboard::focus() const
{
    Q_D(const WaylandKeyboard);
    return d->focus;
}

/*!
 * Sets the current focus to \a surface.
 */
void WaylandKeyboard::setFocus(WaylandSurface *surface)
{
    Q_D(WaylandKeyboard);
    d->focused(surface);
}

/*!
 * \internal
 */
void WaylandKeyboard::addClient(WaylandClient *client, uint32_t id, uint32_t version)
{
    Q_D(WaylandKeyboard);
    d->add(client->client(), id, qMin<uint32_t>(PrivateServer::wl_keyboard::interfaceVersion(), version));
}

uint WaylandKeyboard::keyToScanCode(int qtKey) const
{
    uint scanCode = 0;
#if LIRI_FEATURE_aurora_xkbcommon
    Q_D(const WaylandKeyboard);
    const_cast<WaylandKeyboardPrivate *>(d)->maybeUpdateXkbScanCodeTable();
    scanCode = d->scanCodesByQtKey.value({d->group, qtKey}, 0);
#else
    Q_UNUSED(qtKey);
#endif
    return scanCode;
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandkeyboard.cpp"
