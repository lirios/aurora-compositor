/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "aurorawaylandseat.h"
#include "aurorawaylandseat_p.h"

#include "aurorawaylandcompositor.h"
#include "aurorawaylandinputmethodcontrol.h"
#include "aurorawaylandview.h"
#include <LiriAuroraCompositor/WaylandDrag>
#include <LiriAuroraCompositor/WaylandTouch>
#include <LiriAuroraCompositor/WaylandPointer>
#include <LiriAuroraCompositor/WaylandKeymap>
#include <LiriAuroraCompositor/private/aurorawaylandseat_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandcompositor_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandkeyboard_p.h>
#if LIRI_FEATURE_aurora_datadevice
#include <LiriAuroraCompositor/private/aurorawldatadevice_p.h>
#endif
#include <LiriAuroraCompositor/private/aurorawaylandutils_p.h>

#include "extensions/aurorawlqtkey_p.h"
#include "extensions/aurorawaylandtextinput.h"
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
#  if QT_WAYLAND_TEXT_INPUT_V4_WIP
#    include "extensions/aurorawaylandtextinputv4.h"
#  endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
#  include "extensions/aurorawaylandqttextinputmethod.h"
#endif
#endif

namespace Aurora {

namespace Compositor {

WaylandSeatPrivate::WaylandSeatPrivate(WaylandSeat *seat) :
#if LIRI_FEATURE_aurora_datadevice
    drag_handle(new WaylandDrag(seat)),
#endif
    keymap(new WaylandKeymap())
{
}

WaylandSeatPrivate::~WaylandSeatPrivate()
{
}

void WaylandSeatPrivate::setCapabilities(WaylandSeat::CapabilityFlags caps)
{
    Q_Q(WaylandSeat);
    if (capabilities != caps) {
        WaylandSeat::CapabilityFlags changed = caps ^ capabilities;

        if (changed & WaylandSeat::Pointer) {
            pointer.reset(pointer.isNull() ? WaylandCompositorPrivate::get(compositor)->callCreatePointerDevice(q) : nullptr);
        }

        if (changed & WaylandSeat::Keyboard) {
            keyboard.reset(keyboard.isNull() ? WaylandCompositorPrivate::get(compositor)->callCreateKeyboardDevice(q) : nullptr);
        }

        if (changed & WaylandSeat::Touch) {
            touch.reset(touch.isNull() ? WaylandCompositorPrivate::get(compositor)->callCreateTouchDevice(q) : nullptr);
        }

        capabilities = caps;
        QList<Resource *> resources = resourceMap().values();
        for (int i = 0; i < resources.size(); i++) {
            wl_seat::send_capabilities(resources.at(i)->handle, (uint32_t)capabilities);
        }

        if ((changed & caps & WaylandSeat::Keyboard) && keyboardFocus != nullptr)
            keyboard->setFocus(keyboardFocus);
    }
}

#if LIRI_FEATURE_aurora_datadevice
void WaylandSeatPrivate::clientRequestedDataDevice(QtWayland::DataDeviceManager *, struct wl_client *client, uint32_t id)
{
    Q_Q(WaylandSeat);
    if (!data_device)
        data_device.reset(new QtWayland::DataDevice(q));
    data_device->add(client, id, 1);
}
#endif

void WaylandSeatPrivate::seat_destroy_resource(wl_seat::Resource *)
{
//    cleanupDataDeviceForClient(resource->client(), true);
}

void WaylandSeatPrivate::seat_bind_resource(wl_seat::Resource *resource)
{
    // The order of capabilities matches the order defined in the wayland protocol
    wl_seat::send_capabilities(resource->handle, (uint32_t)capabilities);
}

void WaylandSeatPrivate::seat_get_pointer(wl_seat::Resource *resource, uint32_t id)
{
    if (!pointer.isNull()) {
        pointer->addClient(WaylandClient::fromWlClient(compositor, resource->client()), id, resource->version());
    }
}

void WaylandSeatPrivate::seat_get_keyboard(wl_seat::Resource *resource, uint32_t id)
{
    if (!keyboard.isNull()) {
        keyboard->addClient(WaylandClient::fromWlClient(compositor, resource->client()), id, resource->version());
    }
}

void WaylandSeatPrivate::seat_get_touch(wl_seat::Resource *resource, uint32_t id)
{
    if (!touch.isNull()) {
        touch->addClient(WaylandClient::fromWlClient(compositor, resource->client()), id, resource->version());
    }
}

/*!
 * \qmltype WaylandSeat
 * \instantiates WaylandSeat
 * \inqmlmodule Aurora.Compositor
 * \since 5.8
 * \brief Provides access to keyboard, mouse, and touch input.
 *
 * The WaylandSeat type provides access to different types of user input and maintains
 * a keyboard focus and a mouse pointer. It corresponds to the wl_seat interface in the Wayland
 * protocol.
 */

/*!
 * \class WaylandSeat
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The WaylandSeat class provides access to keyboard, mouse, and touch input.
 *
 * The WaylandSeat provides access to different types of user input and maintains
 * a keyboard focus and a mouse pointer. It corresponds to the wl_seat interface in the Wayland protocol.
 */

/*!
 * \enum WaylandSeat::CapabilityFlag
 *
 * This enum type describes the capabilities of a WaylandSeat.
 *
 * \value Pointer The WaylandSeat supports pointer input.
 * \value Keyboard The WaylandSeat supports keyboard input.
 * \value Touch The WaylandSeat supports touch input.
 * \value DefaultCapabilities The WaylandSeat has the default capabilities.
 */

/*!
 * Constructs a WaylandSeat for the given \a compositor and \a capabilityFlags.
 */
WaylandSeat::WaylandSeat(WaylandCompositor *compositor, CapabilityFlags capabilityFlags)
    : WaylandObject(*new WaylandSeatPrivate(this))
{
    Q_D(WaylandSeat);
    d->compositor = compositor;
    d->capabilities = capabilityFlags;
    if (compositor->isCreated())
        initialize();

    // Support deprecated signal for backward compatibility
    connect(this, &WaylandSeat::cursorSurfaceRequested, this, &WaylandSeat::cursorSurfaceRequest);
}

/*!
 * Destroys the WaylandSeat
 */
WaylandSeat::~WaylandSeat()
{
}

/*!
 * Initializes parts of the seat corresponding to the capabilities set in the constructor, or
 * through setCapabilities().
 *
 * \note Normally, this function is called automatically after the seat and compositor have been
 * created, so calling it manually is usually unnecessary.
 */

void WaylandSeat::initialize()
{
    Q_D(WaylandSeat);
    d->init(d->compositor->display(), 4);

    if (d->capabilities & WaylandSeat::Pointer)
        d->pointer.reset(WaylandCompositorPrivate::get(d->compositor)->callCreatePointerDevice(this));
    if (d->capabilities & WaylandSeat::Touch)
        d->touch.reset(WaylandCompositorPrivate::get(d->compositor)->callCreateTouchDevice(this));
    if (d->capabilities & WaylandSeat::Keyboard)
        d->keyboard.reset(WaylandCompositorPrivate::get(d->compositor)->callCreateKeyboardDevice(this));

    d->isInitialized = true;
}

/*!
 * Returns true if the WaylandSeat is initialized; false otherwise.
 *
 * The value \c true indicates that it's now possible for clients to start using the seat.
 */
bool WaylandSeat::isInitialized() const
{
    Q_D(const WaylandSeat);
    return d->isInitialized;
}

/*!
 * Sends a mouse press event for \a button to the WaylandSeat's pointer device.
 */
void WaylandSeat::sendMousePressEvent(Qt::MouseButton button)
{
    Q_D(WaylandSeat);
    d->pointer->sendMousePressEvent(button);
}

/*!
 * Sends a mouse release event for \a button to the WaylandSeat's pointer device.
 */
void WaylandSeat::sendMouseReleaseEvent(Qt::MouseButton button)
{
    Q_D(WaylandSeat);
    d->pointer->sendMouseReleaseEvent(button);
}

/*!
 * Sets the mouse focus to \a view and sends a mouse move event to the pointer device with the
 * local position \a localPos and output space position \a outputSpacePos.
 **/
void WaylandSeat::sendMouseMoveEvent(WaylandView *view, const QPointF &localPos, const QPointF &outputSpacePos)
{
    Q_D(WaylandSeat);
    d->pointer->sendMouseMoveEvent(view, localPos, outputSpacePos);
}

/*!
 * Sends a mouse wheel event to the WaylandSeat's pointer device with the given \a orientation and \a delta.
 */
void WaylandSeat::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    Q_D(WaylandSeat);
    d->pointer->sendMouseWheelEvent(orientation, delta);
}

/*!
 * Sends a key press event with the key \a code to the keyboard device.
 */
void WaylandSeat::sendKeyPressEvent(uint code)
{
    Q_D(WaylandSeat);
    d->keyboard->sendKeyPressEvent(code);
}

/*!
 * Sends a key release event with the key \a code to the keyboard device.
 */
void WaylandSeat::sendKeyReleaseEvent(uint code)
{
    Q_D(WaylandSeat);
    d->keyboard->sendKeyReleaseEvent(code);
}

/*!
 * Sends a touch point event to the \a surface on a touch device with the given
 * \a id, \a point and \a state.
 *
 * \warning This API should not be used in combination with forwarding of touch
 * events using \l WaylandQuickItem::touchEventsEnabled or \l sendFullTouchEvent,
 * as it might lead to conflicting touch ids.
 *
 * Returns the serial for the touch up or touch down event.
 */
uint WaylandSeat::sendTouchPointEvent(WaylandSurface *surface, int id, const QPointF &point, Qt::TouchPointState state)
{
    Q_D(WaylandSeat);

    if (d->touch.isNull())
        return 0;

    return d->touch->sendTouchPointEvent(surface, id, point,state);
}

/*!
 * \qmlmethod uint QtWaylandCompositor::WaylandSeat::sendTouchPointPressed(WaylandSurface surface, int id, point position)
 *
 * Sends a touch pressed event for the touch point \a id on \a surface with
 * position \a position.
 *
 * \note You need to send a touch frame event when you are done sending touch
 * events.
 *
 * \warning This API should not be used in combination with forwarding of touch
 * events using \l WaylandQuickItem::touchEventsEnabled, as it might lead to
 * conflicting touch ids.
 *
 * Returns the serial for the touch down event.
 */

/*!
 * Sends a touch pressed event for the touch point \a id on \a surface with
 * position \a position.
 *
 * \note You need to send a touch frame event when you are done sending touch
 * events.
 *
 * \warning This API should not be used in combination with forwarding of touch
 * events using \l WaylandQuickItem::touchEventsEnabled or \l sendFullTouchEvent,
 * as it might lead to conflicting touch ids.
 *
 * Returns the serial for the touch down event.
 */
uint WaylandSeat::sendTouchPointPressed(WaylandSurface *surface, int id, const QPointF &position)
{
    return sendTouchPointEvent(surface, id, position, Qt::TouchPointPressed);
}

/*!
 * \qmlmethod void QtWaylandCompositor::WaylandSeat::sendTouchPointReleased(WaylandSurface surface, int id, point position)
 *
 * Sends a touch released event for the touch point \a id on \a surface with
 * position \a position.
 *
 * \note You need to send a touch frame event when you are done sending touch
 * events.
 *
 * \warning This API should not be used in combination with forwarding of touch
 * events using \l WaylandQuickItem::touchEventsEnabled, as it might lead to
 * conflicting touch ids.
 *
 * Returns the serial for the touch up event.
 */

/*!
 * Sends a touch released event for the touch point \a id on \a surface with
 * position \a position.
 *
 * \note You need to send a touch frame event when you are done sending touch
 * events.
 *
 * \warning This API should not be used in combination with forwarding of touch
 * events using \l WaylandQuickItem::touchEventsEnabled or \l sendFullTouchEvent,
 * as it might lead to conflicting touch ids.
 *
 * Returns the serial for the touch up event.
 */
uint WaylandSeat::sendTouchPointReleased(WaylandSurface *surface, int id, const QPointF &position)
{
    return sendTouchPointEvent(surface, id, position, Qt::TouchPointReleased);
}

/*!
 * \qmlmethod void QtWaylandCompositor::WaylandSeat::sendTouchPointMoved(WaylandSurface surface, int id, point position)
 *
 * Sends a touch moved event for the touch point \a id on \a surface with
 * position \a position.
 *
 * \note You need to send a touch frame event when you are done sending touch
 * events.
 *
 * \warning This API should not be used in combination with forwarding of touch
 * events using \l WaylandQuickItem::touchEventsEnabled, as it might lead to
 * conflicting touch ids.
 *
 * Returns the serial for the touch motion event.
 */

/*!
 * Sends a touch moved event for the touch point \a id on \a surface with
 * position \a position.
 *
 * \note You need to send a touch frame event when you are done sending touch
 * events.
 *
 * \warning This API should not be used in combination with forwarding of touch
 * events using \l WaylandQuickItem::touchEventsEnabled or \l sendFullTouchEvent,
 * as it might lead to conflicting touch ids.
 *
 * Returns the serial for the touch motion event.
 */
uint WaylandSeat::sendTouchPointMoved(WaylandSurface *surface, int id, const QPointF &position)
{
    return sendTouchPointEvent(surface, id, position, Qt::TouchPointMoved);
}

/*!
 * \qmlmethod void QtWaylandCompositor::WaylandSeat::sendTouchFrameEvent(WaylandClient client)
 *
 * Sends a frame event to the touch device of a \a client to indicate the end
 * of a series of touch up, down, and motion events.
 */

/*!
 * Sends a frame event to the touch device of a \a client to indicate the end
 * of a series of touch up, down, and motion events.
 */
void WaylandSeat::sendTouchFrameEvent(WaylandClient *client)
{
    Q_D(WaylandSeat);
    if (!d->touch.isNull())
        d->touch->sendFrameEvent(client);
}

/*!
 * \qmlmethod void QtWaylandCompositor::WaylandSeat::sendTouchCancelEvent(WaylandClient client)
 *
 * Sends a cancel event to the touch device of a \a client.
 */

/*!
 * Sends a cancel event to the touch device of a \a client.
 */
void WaylandSeat::sendTouchCancelEvent(WaylandClient *client)
{
    Q_D(WaylandSeat);
    if (!d->touch.isNull())
        d->touch->sendCancelEvent(client);
}

/*!
 * Sends the \a event to the specified \a surface on the touch device.
 *
 * \warning This API will automatically map \l QEventPoint::id() to a
 * sequential id before sending it to the client. It should therefore not be
 * used in combination with the other API using explicit ids, as collisions
 * might occur.
 */
void WaylandSeat::sendFullTouchEvent(WaylandSurface *surface, QTouchEvent *event)
{
    Q_D(WaylandSeat);

    if (!d->touch)
        return;

    d->touch->sendFullTouchEvent(surface, event);
}

/*!
 * Sends the \a event to the keyboard device.
 */
void WaylandSeat::sendFullKeyEvent(QKeyEvent *event)
{
    Q_D(WaylandSeat);

    if (!keyboardFocus()) {
        qWarning("Cannot send key event, no keyboard focus, fix the compositor");
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
    if (keyboardFocus()->inputMethodControl()->enabled()
        && event->nativeScanCode() == 0) {
        if (keyboardFocus()->client()->textInputProtocols().testFlag(WaylandClient::TextInputProtocol::TextInputV2)) {
            WaylandTextInput *textInput = WaylandTextInput::findIn(this);
            if (textInput) {
                textInput->sendKeyEvent(event);
                return;
            }
        }

        if (keyboardFocus()->client()->textInputProtocols().testFlag(WaylandClient::TextInputProtocol::QtTextInputMethodV1)) {
            WaylandQtTextInputMethod *textInputMethod = WaylandQtTextInputMethod::findIn(this);
            if (textInputMethod) {
                textInputMethod->sendKeyEvent(event);
                return;
            }
        }

#if QT_WAYLAND_TEXT_INPUT_V4_WIP
        if (keyboardFocus()->client()->textInputProtocols().testFlag(WaylandClient::TextInputProtocol::TextInputV4)) {
            WaylandTextInputV4 *textInputV4 = WaylandTextInputV4::findIn(this);
            if (textInputV4 && !event->text().isEmpty()) {
                // it will just commit the text for text-input-unstable-v4-wip when keyPress
                if (event->type() == QEvent::KeyPress)
                    textInputV4->sendKeyEvent(event);
                return;
            }
        }
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
    }
#endif
#endif

    QtWayland::QtKeyExtensionGlobal *ext = QtWayland::QtKeyExtensionGlobal::findIn(d->compositor);
    if (ext && ext->postQtKeyEvent(event, keyboardFocus()))
        return;

    if (!d->keyboard.isNull() && !event->isAutoRepeat()) {

        uint scanCode = event->nativeScanCode();
        if (scanCode == 0)
            scanCode = d->keyboard->keyToScanCode(event->key());

        if (scanCode == 0) {
            qWarning() << "Can't send Wayland key event: Unable to get a valid scan code";
            return;
        }

        if (event->type() == QEvent::KeyPress) {
            WaylandKeyboardPrivate::get(d->keyboard.data())->checkAndRepairModifierState(event);
            d->keyboard->sendKeyPressEvent(scanCode);
        } else if (event->type() == QEvent::KeyRelease) {
            d->keyboard->sendKeyReleaseEvent(scanCode);
        }
    }
}

/*!
 * \qmlmethod void QtWaylandCompositor::WaylandSeat::sendKeyEvent(int qtKey, bool pressed)
 * \since 5.12
 *
 * Sends a key press (if \a pressed is \c true) or release (if \a pressed is \c false)
 * event of a key \a qtKey to the keyboard device.
 */

/*!
 * Sends a key press (if \a pressed is \c true) or release (if \a pressed is \c false)
 * event of a key \a qtKey to the keyboard device.
 *
 * \since 5.12
 */
void WaylandSeat::sendKeyEvent(int qtKey, bool pressed)
{
    Q_D(WaylandSeat);
    if (!keyboardFocus()) {
        qWarning("Cannot send Wayland key event, no keyboard focus, fix the compositor");
        return;
    }

    if (auto scanCode = d->keyboard->keyToScanCode(qtKey)) {
        if (pressed)
            d->keyboard->sendKeyPressEvent(scanCode);
        else
            d->keyboard->sendKeyReleaseEvent(scanCode);
    } else {
        qWarning() << "Can't send Wayland key event: Unable to get scan code for" << Qt::Key(qtKey);
    }
}

/*!
 * Returns the keyboard for this input device.
 */
WaylandKeyboard *WaylandSeat::keyboard() const
{
    Q_D(const WaylandSeat);
    return d->keyboard.data();
}

/*!
 * Returns the current focused surface for keyboard input.
 */
WaylandSurface *WaylandSeat::keyboardFocus() const
{
    Q_D(const WaylandSeat);
    if (d->keyboard.isNull() || !d->keyboard->focus())
        return nullptr;

    return d->keyboard->focus();
}

/*!
 * Sets the current keyboard focus to \a surface.
 * Returns a boolean indicating if the operation
 * was successful.
 */
bool WaylandSeat::setKeyboardFocus(WaylandSurface *surface)
{
    Q_D(WaylandSeat);
    if (surface && surface->isDestroyed())
        return false;

    WaylandSurface *oldSurface = keyboardFocus();
    if (surface == oldSurface)
        return true;

    d->keyboardFocus = surface;
    if (!d->keyboard.isNull())
        d->keyboard->setFocus(surface);
#if LIRI_FEATURE_aurora_datadevice
    if (d->data_device)
        d->data_device->setFocus(surface ? surface->client() : nullptr);
#endif
    emit keyboardFocusChanged(surface, oldSurface);
    return true;
}


/*!
 * Returns the keymap object for this WaylandSeat.
 */

WaylandKeymap *WaylandSeat::keymap()
{
    Q_D(const WaylandSeat);
    return d->keymap.data();
}

/*!
 * Returns the pointer device for this WaylandSeat.
 */
WaylandPointer *WaylandSeat::pointer() const
{
    Q_D(const WaylandSeat);
    return d->pointer.data();
}

/*!
 * Returns the touch device for this WaylandSeat.
 */
WaylandTouch *WaylandSeat::touch() const
{
    Q_D(const WaylandSeat);
    return d->touch.data();
}

/*!
 * Returns the view that currently has mouse focus.
 */
WaylandView *WaylandSeat::mouseFocus() const
{
    Q_D(const WaylandSeat);
    return d->mouseFocus;
}

/*!
 * Sets the current mouse focus to \a view.
 */
void WaylandSeat::setMouseFocus(WaylandView *view)
{
    Q_D(WaylandSeat);
    if (view == d->mouseFocus)
        return;

    WaylandView *oldFocus = d->mouseFocus;
    d->mouseFocus = view;

    if (oldFocus)
        disconnect(oldFocus, &QObject::destroyed, this, &WaylandSeat::handleMouseFocusDestroyed);
    if (d->mouseFocus)
        connect(d->mouseFocus, &QObject::destroyed, this, &WaylandSeat::handleMouseFocusDestroyed);

    emit mouseFocusChanged(d->mouseFocus, oldFocus);
}

/*!
 * Returns the compositor for this WaylandSeat.
 */
WaylandCompositor *WaylandSeat::compositor() const
{
    Q_D(const WaylandSeat);
    return d->compositor;
}

/*!
 * Returns the drag object for this WaylandSeat.
 */
WaylandDrag *WaylandSeat::drag() const
{
    Q_D(const WaylandSeat);
    return d->drag_handle.data();
}

/*!
 * Returns the capability flags for this WaylandSeat.
 */
WaylandSeat::CapabilityFlags WaylandSeat::capabilities() const
{
    Q_D(const WaylandSeat);
    return d->capabilities;
}

/*!
 * \internal
 */
bool WaylandSeat::isOwner(QInputEvent *inputEvent) const
{
    Q_UNUSED(inputEvent);
    return true;
}

/*!
 * Returns the WaylandSeat corresponding to the \a resource. The \a resource is expected
 * to have the type wl_seat.
 */
WaylandSeat *WaylandSeat::fromSeatResource(struct ::wl_resource *resource)
{
    if (auto p = QtWayland::fromResource<WaylandSeatPrivate *>(resource))
        return p->q_func();
    return nullptr;
}

/*!
 * \fn void WaylandSeat::mouseFocusChanged(WaylandView *newFocus, WaylandView *oldFocus)
 *
 * This signal is emitted when the mouse focus has changed from \a oldFocus to \a newFocus.
 */

void WaylandSeat::handleMouseFocusDestroyed()
{
    // This is triggered when the WaylandView is destroyed, NOT the surface.
    // ... so this is for the rare case when the view that currently holds the mouse focus is
    // destroyed before its surface
    Q_D(WaylandSeat);
    d->mouseFocus = nullptr;
    WaylandView *oldFocus = nullptr; // we have to send nullptr because the old focus is already destroyed at this point
    emit mouseFocusChanged(d->mouseFocus, oldFocus);
}


/*! \qmlsignal void QtWaylandCompositor::WaylandSeat::keyboardFocusChanged(WaylandSurface newFocus, WaylandSurface oldFocus)
 *
 * This signal is emitted when setKeyboardFocus() is called or when a WaylandQuickItem has focus
 * and the user starts pressing keys.
 *
 * \a newFocus has the surface that received keyboard focus; or \c nullptr if no surface has
 * focus.
 * \a oldFocus has the surface that lost keyboard focus; or \c nullptr if no surface had focus.
 */

/*!
 * \fn void WaylandSeat::keyboardFocusChanged(WaylandSurface *newFocus, WaylandSurface *oldFocus)
 *
 * This signal is emitted when setKeyboardFocus() is called.
 *
 * \a newFocus has the surface that received keyboard focus; or \c nullptr if no surface has
 * focus.
 * \a oldFocus has the surface that lost keyboard focus; or \c nullptr if no surface had focus.
 */

/*! \qmlsignal void QtWaylandCompositor::WaylandSeat::cursorSurfaceRequest(WaylandSurface surface, int hotspotX, int hotspotY)
 *
 * This signal is emitted when the client has requested for a specific \a surface to be the mouse
 * cursor. For example, when the user hovers over a particular surface, and you want the cursor
 * to change into a resize arrow.
 *
 * Both \a hotspotX and \a hotspotY are offsets from the top-left of a pointer surface, where a
 * click should happen. For example, if the requested cursor surface is an arrow, the parameters
 * indicate where the arrow's tip is, on that surface.
 */


/*!
 * \fn void WaylandSeat::cursorSurfaceRequest(WaylandSurface *surface, int hotspotX, int hotspotY)
 *
 * This signal is emitted when the client has requested for a specific \a surface to be the mouse
 * cursor. For example, when the user hovers over a particular surface, and you want the cursor
 * to change into a resize arrow.
 *
 * Both \a hotspotX and \a hotspotY are offsets from the top-left of a pointer surface, where a
 * click should happen. For example, if the requested cursor surface is an arrow, the parameters
 * indicate where the arrow's tip is, on that surface.
 */

/*!
 * \property WaylandSeat::drag
 *
 * This property holds the drag and drop operations and sends signals when they start and end.
 * The property stores details like what image should be under the mouse cursor when the user
 * drags it.
 */

/*!
 * \property WaylandSeat::keymap
 * This property holds the keymap object.
 *
 * A keymap provides a way to translate actual key scan codes into a meaningful value.
 * For example, if you use a keymap with a Norwegian layout, the key to the right of
 * the letter L produces an Ø.
 *
 * Keymaps can also be used to customize key functions, such as to specify whether
 * Control and CAPS lock should be swapped, and so on.
 */

} // namespace Compositor

} // namespace Aurora
