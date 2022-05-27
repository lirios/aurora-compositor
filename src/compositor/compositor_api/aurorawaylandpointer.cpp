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

#include "aurorawaylandpointer.h"
#include "aurorawaylandpointer_p.h"
#include <LiriAuroraCompositor/WaylandClient>
#include <LiriAuroraCompositor/WaylandCompositor>

namespace Aurora {

namespace Compositor {

WaylandSurfaceRole WaylandPointerPrivate::s_role("wl_pointer");

WaylandPointerPrivate::WaylandPointerPrivate(WaylandPointer *pointer, WaylandSeat *seat)
    : q_ptr(pointer)
    , seat(seat)
{
}

uint WaylandPointerPrivate::sendButton(Qt::MouseButton button, uint32_t state)
{
    Q_Q(WaylandPointer);
    if (!q->mouseFocus() || !q->mouseFocus()->surface())
        return 0;

    if (!seat->isInputAllowed(q->mouseFocus()->surface()))
        return 0;

    wl_client *client = q->mouseFocus()->surface()->waylandClient();
    uint32_t time = compositor()->currentTimeMsecs();
    uint32_t serial = compositor()->nextSerial();
    for (auto resource : resourceMap().values(client))
        send_button(resource->handle, serial, time, q->toWaylandButton(button), state);
    return serial;
}

void WaylandPointerPrivate::sendMotion()
{
    Q_ASSERT(enteredSurface);
    uint32_t time = compositor()->currentTimeMsecs();
    wl_fixed_t x = wl_fixed_from_double(localPosition.x());
    wl_fixed_t y = wl_fixed_from_double(localPosition.y());
    for (auto resource : resourceMap().values(enteredSurface->waylandClient()))
        wl_pointer_send_motion(resource->handle, time, x, y);
}

void WaylandPointerPrivate::sendEnter(WaylandSurface *surface)
{
    Q_ASSERT(surface && !enteredSurface);
    enterSerial = compositor()->nextSerial();

    WaylandKeyboard *keyboard = seat->keyboard();
    if (keyboard)
        keyboard->sendKeyModifiers(surface->client(), enterSerial);

    wl_fixed_t x = wl_fixed_from_double(localPosition.x());
    wl_fixed_t y = wl_fixed_from_double(localPosition.y());
    for (auto resource : resourceMap().values(surface->waylandClient()))
        send_enter(resource->handle, enterSerial, surface->resource(), x, y);

    enteredSurface = surface;
    enteredSurfaceDestroyListener.listenForDestruction(surface->resource());
}

void WaylandPointerPrivate::sendLeave()
{
    Q_ASSERT(enteredSurface);
    uint32_t serial = compositor()->nextSerial();
    for (auto resource : resourceMap().values(enteredSurface->waylandClient()))
        send_leave(resource->handle, serial, enteredSurface->resource());
    localPosition = QPointF();
    enteredSurfaceDestroyListener.reset();
    enteredSurface = nullptr;
}

void WaylandPointerPrivate::ensureEntered(WaylandSurface *surface)
{
    if (enteredSurface == surface)
        return;

    if (enteredSurface)
        sendLeave();

    if (surface)
        sendEnter(surface);
}

void WaylandPointerPrivate::pointer_release(wl_pointer::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void WaylandPointerPrivate::pointer_set_cursor(wl_pointer::Resource *resource, uint32_t serial, wl_resource *surface, int32_t hotspot_x, int32_t hotspot_y)
{
    Q_UNUSED(serial);

    if (!surface) {
        emit seat->cursorSurfaceRequested(nullptr, 0, 0, WaylandClient::fromWlClient(compositor(), resource->client()));
        return;
    }

    WaylandSurface *s = WaylandSurface::fromResource(surface);
    // XXX FIXME
    // The role concept was formalized in wayland 1.7, so that release adds one error
    // code for each interface that implements a role, and we are supposed to pass here
    // the newly constructed resource and the correct error code so that if setting the
    // role fails, a proper error can be sent to the client.
    // However we're still using wayland 1.4, which doesn't have interface specific role
    // errors, so the best we can do is to use wl_display's object_id error.
    wl_resource *displayRes = wl_client_get_object(resource->client(), 1);
    if (s->setRole(&WaylandPointerPrivate::s_role, displayRes, WL_DISPLAY_ERROR_INVALID_OBJECT)) {
        s->markAsCursorSurface(true);
        emit seat->cursorSurfaceRequested(s, hotspot_x, hotspot_y, WaylandClient::fromWlClient(compositor(), resource->client()));
    }
}

/*!
 * \class WaylandPointer
 * \inmodule AuroraCompositor
 * \since 5.8
 * \brief The WaylandPointer class represents a pointer device.
 *
 * This class provides access to the pointer device in a WaylandSeat. It corresponds to
 * the Wayland interface wl_pointer.
 */

/*!
 * Constructs a WaylandPointer for the given \a seat and with the given \a parent.
 */
WaylandPointer::WaylandPointer(WaylandSeat *seat, QObject *parent)
    : WaylandObject(parent)
    , d_ptr(new WaylandPointerPrivate(this, seat))
{
    connect(&d_func()->enteredSurfaceDestroyListener, &WaylandDestroyListener::fired, this, &WaylandPointer::enteredSurfaceDestroyed);
    connect(seat, &WaylandSeat::mouseFocusChanged, this, &WaylandPointer::pointerFocusChanged);
}

WaylandPointer::~WaylandPointer()
{
}

/*!
 * Returns the input device for this WaylandPointer.
 */
WaylandSeat *WaylandPointer::seat() const
{
    Q_D(const WaylandPointer);
    return d->seat;
}

/*!
 * Returns the compositor for this WaylandPointer.
 */
WaylandCompositor *WaylandPointer::compositor() const
{
    Q_D(const WaylandPointer);
    return d->compositor();
}

/*!
 * Returns the output for this WaylandPointer.
 */
WaylandOutput *WaylandPointer::output() const
{
    Q_D(const WaylandPointer);
    return d->output;
}

/*!
 * Sets the output for this WaylandPointer to \a output.
 */
void WaylandPointer::setOutput(WaylandOutput *output)
{
    Q_D(WaylandPointer);
    if (d->output == output) return;
    d->output = output;
    emit outputChanged();
}

/*!
 * Sends a mouse press event for \a button to the view currently holding mouse focus.
 *
 * Returns the serial number of the press event.
 */
uint WaylandPointer::sendMousePressEvent(Qt::MouseButton button)
{
    Q_D(WaylandPointer);
    d->buttonCount++;

    if (d->buttonCount == 1)
        emit buttonPressedChanged();

    return d->sendButton(button, WL_POINTER_BUTTON_STATE_PRESSED);
}

/*!
 * Sends a mouse release event for \a button to the view currently holding mouse focus.
 *
 * Returns the serial number of the release event.
 */
uint WaylandPointer::sendMouseReleaseEvent(Qt::MouseButton button)
{
    Q_D(WaylandPointer);
    d->buttonCount--;

    if (d->buttonCount == 0)
        emit buttonPressedChanged();

    return d->sendButton(button, WL_POINTER_BUTTON_STATE_RELEASED);
}

/*!
 * Sets the current mouse focus to \a view and sends a mouse move event to it with the
 * local position \a localPos in surface coordinates and output space position \a outputSpacePos.
 */
void WaylandPointer::sendMouseMoveEvent(WaylandView *view, const QPointF &localPos, const QPointF &outputSpacePos)
{
    Q_D(WaylandPointer);
    if (view && (!view->surface() || view->surface()->isCursorSurface()))
        view = nullptr;
    d->seat->setMouseFocus(view);
    d->localPosition = localPos;
    d->spacePosition = outputSpacePos;

    if (view) {
        // We adjust if the mouse position is on the edge
        // to work around Qt's event propagation
        QSizeF size(view->surface()->destinationSize());
        if (d->localPosition.x() == size.width())
            d->localPosition.rx() -= 0.01;
        if (d->localPosition.y() == size.height())
            d->localPosition.ry() -= 0.01;

        d->ensureEntered(view->surface());
        d->sendMotion();

        if (view->output())
            setOutput(view->output());
    }
}

/*!
 * Sends a mouse wheel event with the given \a orientation and \a delta to the view that currently holds mouse focus.
 */
void WaylandPointer::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    Q_D(WaylandPointer);
    if (!d->enteredSurface)
        return;

    if (!d->seat->isInputAllowed(d->enteredSurface))
        return;

    uint32_t time = d->compositor()->currentTimeMsecs();
    uint32_t axis = orientation == Qt::Horizontal ? WL_POINTER_AXIS_HORIZONTAL_SCROLL
                                                  : WL_POINTER_AXIS_VERTICAL_SCROLL;

    for (auto resource : d->resourceMap().values(d->enteredSurface->waylandClient()))
        d->send_axis(resource->handle, time, axis, wl_fixed_from_int(-delta / 12));
}

/*!
 * Returns the view that currently holds mouse focus.
 */
WaylandView *WaylandPointer::mouseFocus() const
{
    Q_D(const WaylandPointer);
    return d->seat->mouseFocus();
}

/*!
 * Returns the current local position of the WaylandPointer in surface coordinates.
 */
QPointF WaylandPointer::currentLocalPosition() const
{
    Q_D(const WaylandPointer);
    return d->localPosition;
}

/*!
 * Returns the current output space position of the WaylandPointer.
 */
QPointF WaylandPointer::currentSpacePosition() const
{
    Q_D(const WaylandPointer);
    return d->spacePosition;
}

/*!
 * Returns true if any button is currently pressed. Otherwise returns false.
 */
bool WaylandPointer::isButtonPressed() const
{
    Q_D(const WaylandPointer);
    return d->buttonCount > 0;
}

/*!
 * \internal
 */
void WaylandPointer::addClient(WaylandClient *client, uint32_t id, uint32_t version)
{
    Q_D(WaylandPointer);
    wl_resource *resource = d->add(client->client(), id, qMin<uint32_t>(PrivateServer::wl_pointer::interfaceVersion(), version))->handle;
    if (d->enteredSurface && client == d->enteredSurface->client()) {
        d->send_enter(resource, d->enterSerial, d->enteredSurface->resource(),
                      wl_fixed_from_double(d->localPosition.x()),
                      wl_fixed_from_double(d->localPosition.y()));
    }
}

/*!
 * Returns a Wayland resource for this WaylandPointer.
 *
 * This API doesn't actually make sense, since there may be many pointer resources per client
 * It's here for compatibility reasons.
 */
struct wl_resource *WaylandPointer::focusResource() const
{
    Q_D(const WaylandPointer);
    WaylandView *focus = d->seat->mouseFocus();
    if (!focus)
        return nullptr;

    // Just return the first resource we can find.
    return d->resourceMap().value(focus->surface()->waylandClient())->handle;
}

/*!
 * \internal
 */
uint WaylandPointer::sendButton(struct wl_resource *resource, uint32_t time, Qt::MouseButton button, uint32_t state)
{
    // This method is here for compatibility reasons only, since it usually doesn't make sense to
    // send button events to just one of the pointer resources for a client.
    Q_D(WaylandPointer);
    uint32_t serial = d->compositor()->nextSerial();
    d->send_button(resource, serial, time, toWaylandButton(button), state);
    return serial;
}

/*!
 * \internal
 */
uint32_t WaylandPointer::toWaylandButton(Qt::MouseButton button)
{
#ifndef BTN_LEFT
    uint32_t BTN_LEFT = 0x110;
#endif
    // the range of valid buttons (evdev module) is from 0x110
    // through 0x11f. 0x120 is the first 'Joystick' button.
    switch (button) {
    case Qt::LeftButton: return BTN_LEFT;
    case Qt::RightButton: return uint32_t(0x111);
    case Qt::MiddleButton: return uint32_t(0x112);
    case Qt::ExtraButton1: return uint32_t(0x113);  // AKA Qt::BackButton, Qt::XButton1
    case Qt::ExtraButton2: return uint32_t(0x114);  // AKA Qt::ForwardButton, Qt::XButton2
    case Qt::ExtraButton3: return uint32_t(0x115);
    case Qt::ExtraButton4: return uint32_t(0x116);
    case Qt::ExtraButton5: return uint32_t(0x117);
    case Qt::ExtraButton6: return uint32_t(0x118);
    case Qt::ExtraButton7: return uint32_t(0x119);
    case Qt::ExtraButton8: return uint32_t(0x11a);
    case Qt::ExtraButton9: return uint32_t(0x11b);
    case Qt::ExtraButton10: return uint32_t(0x11c);
    case Qt::ExtraButton11: return uint32_t(0x11d);
    case Qt::ExtraButton12: return uint32_t(0x11e);
    case Qt::ExtraButton13: return uint32_t(0x11f);
        // default should not occur; but if it does, then return Wayland's highest possible button number.
    default: return uint32_t(0x11f);
    }
}

/*!
 * \internal
 */
void WaylandPointer::enteredSurfaceDestroyed(void *data)
{
    Q_D(WaylandPointer);
    Q_UNUSED(data);
    d->enteredSurfaceDestroyListener.reset();
    d->enteredSurface = nullptr;

    d->seat->setMouseFocus(nullptr);

    if (d->buttonCount != 0) {
        d->buttonCount = 0;
        emit buttonPressedChanged();
    }
}

/*!
 * \internal
 */
void WaylandPointer::pointerFocusChanged(WaylandView *newFocus, WaylandView *oldFocus)
{
    Q_D(WaylandPointer);
    Q_UNUSED(oldFocus);
    bool wasSameSurface = newFocus && newFocus->surface() == d->enteredSurface;
    if (d->enteredSurface && !wasSameSurface)
        d->sendLeave();
}

} // namespace Compositor

} // namespace Aurora
