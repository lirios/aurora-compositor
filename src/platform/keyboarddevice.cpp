// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "auroraplatformloggingcategories.h"
#include "keyboarddevice.h"
#include "keyboarddevice_p.h"

#include <sys/mman.h>
#include <unistd.h>

namespace Aurora {

namespace Platform {

/*
 * KeyboardDevicePrivate
 */

KeyboardDevicePrivate::KeyboardDevicePrivate(KeyboardDevice *self)
    : q_ptr(self)
{
    xkbContext.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));
    if (!xkbContext)
        qCWarning(gLcAuroraPlatform) << "Unable to create xkb context";
}

#if LIRI_FEATURE_aurora_xkbcommon
bool KeyboardDevicePrivate::createDefaultKeymap()
{
    if (!xkbContext)
        return false;

    struct xkb_rule_names names;
    names.rules = "evdev";
    names.model = "pc105";
    names.layout = "us";
    names.variant = "";
    names.options = "";

    xkbKeymap.reset(
            xkb_keymap_new_from_names(xkbContext.get(), &names, XKB_KEYMAP_COMPILE_NO_FLAGS));
    if (xkbKeymap)
        xkbState.reset(xkb_state_new(xkbKeymap.get()));

    if (!xkbKeymap || !xkbState) {
        qCWarning(gLcAuroraPlatform) << "Failed to create default keymap";
        return false;
    }

    return true;
}
#endif

/*
 * KeyboardDevice
 */

KeyboardDevice::KeyboardDevice(QObject *parent)
    : InputDevice(parent)
    , d_ptr(new KeyboardDevicePrivate(this))
{
    qRegisterMetaType<KeyEvent>("KeyboardDevice::KeyEvent");

    Q_D(KeyboardDevice);
    d->repeatTimer.setSingleShot(true);
    d->repeatTimer.callOnTimeout(this, [this, d]() {
        KeyEvent keyEvent = { d->repeatKey.key,
                              d->repeatKey.modifiers,
                              d->repeatKey.code,
                              d->repeatKey.nativeVirtualKey,
                              d->repeatKey.nativeModifiers,
                              d->repeatKey.text,
                              d->repeatKey.time,
                              true,
                              d->repeatKey.repeatCount };
        Q_EMIT keyReleased(keyEvent);
        Q_EMIT keyPressed(keyEvent);

        ++d->repeatKey.repeatCount;
        d->repeatTimer.setInterval(d->keyRepeatRate);
        d->repeatTimer.start();
    });
}

KeyboardDevice::~KeyboardDevice()
{
}

InputDevice::DeviceType KeyboardDevice::deviceType()
{
    return DeviceType::Keyboard;
}

bool KeyboardDevice::isKeyRepeatEnabled() const
{
    return false;
}

qint32 KeyboardDevice::keyRepeatRate() const
{
    Q_D(const KeyboardDevice);
    return d->keyRepeatRate;
}

qint32 KeyboardDevice::keyRepeatDelay() const
{
    Q_D(const KeyboardDevice);
    return d->keyRepeatDelay;
}

Qt::KeyboardModifiers KeyboardDevice::modifiers() const
{
    Q_D(const KeyboardDevice);

#if LIRI_FEATURE_aurora_xkbcommon
    if (!d->xkbState)
        return Qt::NoModifier;
    return PlatformSupport::XkbCommon::modifiers(d->xkbState.get());
#else
    return Qt::NoModifier;
#endif
}

void KeyboardDevice::setKeyRepeatEnabled(bool enabled)
{
    Q_D(KeyboardDevice);

    if (d->keyRepeatEnabled != enabled) {
        d->keyRepeatEnabled = enabled;
        Q_EMIT keyRepeatEnabledChanged(enabled);

        if (!enabled)
            d->repeatTimer.stop();
    }
}

void KeyboardDevice::setKeyRepeatRate(qint32 value)
{
    Q_D(KeyboardDevice);

    if (d->keyRepeatRate != value) {
        d->keyRepeatRate = value;
        Q_EMIT keyRepeatRateChanged(value);
    }
}

void KeyboardDevice::setKeyRepeatDelay(qint32 value)
{
    Q_D(KeyboardDevice);

    if (d->keyRepeatDelay != value) {
        d->keyRepeatDelay = value;
        Q_EMIT keyRepeatDelayChanged(value);
    }
}

void KeyboardDevice::handleKeymapChanged(int fd, quint32 size)
{
#if LIRI_FEATURE_aurora_xkbcommon
    Q_D(KeyboardDevice);

    char *map_str = static_cast<char *>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (map_str == MAP_FAILED) {
        close(fd);
        return;
    }

    d->xkbKeymap.reset(xkb_keymap_new_from_string(
            d->xkbContext.get(), map_str, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS));
    PlatformSupport::XkbCommon::verifyHasLatinLayout(d->xkbKeymap.get());

    munmap(map_str, size);
    close(fd);

    if (d->xkbKeymap)
        d->xkbState.reset(xkb_state_new(d->xkbKeymap.get()));
    else
        d->xkbState.reset(nullptr);
#else
    Q_UNUSED(fd)
    Q_UNUSED(size)
#endif
}

void KeyboardDevice::handleKeyChanged(quint32 key, const KeyState &keyState, quint32 time)
{
#if LIRI_FEATURE_aurora_xkbcommon
    Q_D(KeyboardDevice);

    // Make sure we have a keymap loaded
    if (!d->xkbKeymap || !d->xkbState) {
        if (!d->createDefaultKeymap())
            return;
    }

    // Sanity check
    if (!d->xkbKeymap || !d->xkbState)
        return;

    const auto isPressed = keyState == KeyboardDevice::KeyState::Pressed;

    auto code = key + 8;
    auto qtModifiers = modifiers();
    auto sym = xkb_state_key_get_one_sym(d->xkbState.get(), code);
    auto qtKey =
            PlatformSupport::XkbCommon::keysymToQtKey(sym, modifiers(), d->xkbState.get(), code);
    auto text = PlatformSupport::XkbCommon::lookupString(d->xkbState.get(), code);

    xkb_state_update_key(d->xkbState.get(), code, isPressed ? XKB_KEY_DOWN : XKB_KEY_UP);

    KeyEvent keyEvent = { qtKey, qtModifiers, code, sym, d->nativeModifiers, text, time, false, 1 };
    if (isPressed)
        Q_EMIT keyPressed(keyEvent);
    else if (keyState == KeyboardDevice::KeyState::Released)
        Q_EMIT keyReleased(keyEvent);

    if (isPressed && d->keyRepeatEnabled && d->keyRepeatRate > 0
        && xkb_keymap_key_repeats(d->xkbKeymap.get(), code)) {
        d->repeatKey.key = qtKey;
        d->repeatKey.code = code;
        d->repeatKey.time = time;
        d->repeatKey.text = text;
        d->repeatKey.modifiers = qtModifiers;
        d->repeatKey.nativeModifiers = d->nativeModifiers;
        d->repeatKey.nativeVirtualKey = sym;
        d->repeatKey.repeatCount = 1;
        d->repeatTimer.setInterval(d->keyRepeatDelay);
        d->repeatTimer.start();
    } else if (d->repeatTimer.isActive()) {
        d->repeatTimer.stop();
    }
#else
    Q_UNUSED(key)
    Q_UNUSED(keyState)
    Q_UNUSED(time)
#endif
}

void KeyboardDevice::handleModifiers(quint32 depressed, quint32 latched, quint32 locked,
                                     quint32 group)
{
#if LIRI_FEATURE_aurora_xkbcommon
    Q_D(KeyboardDevice);

    if (d->xkbState)
        xkb_state_update_mask(d->xkbState.get(), depressed, latched, locked, 0, 0, group);
    d->nativeModifiers = depressed | latched | locked;
#else
    Q_UNUSED(depressed)
    Q_UNUSED(latched)
    Q_UNUSED(locked)
    Q_UNUSED(group)
#endif
}

} // namespace Platform

} // namespace Aurora
