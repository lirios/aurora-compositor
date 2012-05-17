/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "wlinputdevice.h"

#include "wlcompositor.h"
#include "wldatadevice.h"
#include "wlsurface.h"
#include "wltouch.h"
#include "wlqtkey.h"
#include "waylandcompositor.h"

#include <QtGui/QTouchEvent>

namespace Wayland {

static QImage *currentCursor;

InputDevice::InputDevice(WaylandInputDevice *handle, Compositor *compositor)
    : m_handle(handle)
    , m_compositor(compositor)
{
    wl_seat_init(base());
    initDevices();
    wl_display_add_global(compositor->wl_display(),
                          &wl_seat_interface,
                          this,
                          InputDevice::bind_func);
}

InputDevice::~InputDevice()
{
    qDeleteAll(m_data_devices);
    releaseDevices();
}

void InputDevice::initDevices()
{
    wl_pointer_init(&m_device_interfaces.pointer);
    wl_seat_set_pointer(base(), &m_device_interfaces.pointer);

    wl_keyboard_init(&m_device_interfaces.keyboard);
    wl_seat_set_keyboard(base(), &m_device_interfaces.keyboard);

    wl_touch_init(&m_device_interfaces.touch);
    wl_seat_set_touch(base(), &m_device_interfaces.touch);
}

void InputDevice::releaseDevices()
{
    wl_pointer_release(&m_device_interfaces.pointer);
    wl_keyboard_release(&m_device_interfaces.keyboard);
    wl_touch_release(&m_device_interfaces.touch);
}

wl_pointer *InputDevice::pointerDevice()
{
    return &m_device_interfaces.pointer;
}

wl_keyboard *InputDevice::keyboardDevice()
{
    return &m_device_interfaces.keyboard;
}

wl_touch *InputDevice::touchDevice()
{
    return &m_device_interfaces.touch;
}

const wl_pointer *InputDevice::pointerDevice() const
{
    return &m_device_interfaces.pointer;
}

const wl_keyboard *InputDevice::keyboardDevice() const
{
    return &m_device_interfaces.keyboard;
}

const wl_touch *InputDevice::touchDevice() const
{
    return &m_device_interfaces.touch;
}

void InputDevice::destroy_resource(wl_resource *resource)
{
    InputDevice *input_device = static_cast<InputDevice *>(resource->data);
    if (input_device->keyboardDevice()->focus_resource == resource) {
        input_device->keyboardDevice()->focus_resource = 0;
    }
    if (input_device->pointerDevice()->focus_resource == resource) {
        input_device->pointerDevice()->focus_resource = 0;
    }

    input_device->cleanupDataDeviceForClient(resource->client, true);

    wl_list_remove(&resource->link);

    free(resource);
}

void InputDevice::bind_func(struct wl_client *client, void *data,
                            uint32_t version, uint32_t id)
{
    Q_UNUSED(version);
    struct wl_resource *resource = wl_client_add_object(client,
                                                        &wl_seat_interface,
                                                        &seat_interface,
                                                        id,
                                                        data);

    struct wl_seat *seat = static_cast<struct wl_seat *>(data);
    resource->destroy = destroy_resource;
    wl_list_insert(&seat->base_resource_list, &resource->link);

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER | WL_SEAT_CAPABILITY_KEYBOARD;
    if (!QTouchDevice::devices().isEmpty())
        caps |= WL_SEAT_CAPABILITY_TOUCH;

    wl_seat_send_capabilities(resource, caps);
}

const struct wl_seat_interface InputDevice::seat_interface = {
    get_pointer,
    get_keyboard,
    get_touch
};

void InputDevice::destroy_device_resource(wl_resource *resource)
{
    wl_list_remove(&resource->link);
    free(resource);
}

void InputDevice::get_pointer(struct wl_client *client,
                              struct wl_resource *resource,
                              uint32_t id)
{
    InputDevice *inputDevice = static_cast<InputDevice *>(resource->data);
    wl_pointer *pointer = inputDevice->pointerDevice();
    wl_resource *clientResource = wl_client_add_object(client,
                                                       &wl_pointer_interface,
                                                       &pointer_interface,
                                                       id,
                                                       pointer);
    wl_list_insert(&pointer->resource_list, &clientResource->link);
    clientResource->destroy = InputDevice::destroy_device_resource;
}

void InputDevice::get_keyboard(struct wl_client *client,
                               struct wl_resource *resource,
                               uint32_t id)
{
    InputDevice *inputDevice = static_cast<InputDevice *>(resource->data);
    wl_keyboard *keyboard = inputDevice->keyboardDevice();
    wl_resource *clientResource = wl_client_add_object(client,
                                                       &wl_keyboard_interface,
                                                       0,
                                                       id,
                                                       keyboard);
    wl_list_insert(&keyboard->resource_list, &clientResource->link);
    clientResource->destroy = InputDevice::destroy_device_resource;
}

void InputDevice::get_touch(struct wl_client *client,
                            struct wl_resource *resource,
                            uint32_t id)
{
    InputDevice *inputDevice = static_cast<InputDevice *>(resource->data);
    wl_touch *touch = inputDevice->touchDevice();
    wl_resource *clientResource = wl_client_add_object(client,
                                                       &wl_touch_interface,
                                                       0,
                                                       id,
                                                       touch);
    wl_list_insert(&touch->resource_list, &clientResource->link);
    clientResource->destroy = InputDevice::destroy_device_resource;
}

void InputDevice::sendMousePressEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    sendMouseMoveEvent(localPos,globalPos);
    wl_pointer *pointer = pointerDevice();
    pointer->button_count++;
    uint32_t time = m_compositor->currentTimeMsecs();
    const struct wl_pointer_grab_interface *interface = pointer->grab->interface;
    interface->button(pointer->grab, time, toWaylandButton(button), 1);
}

void InputDevice::sendMouseReleaseEvent(Qt::MouseButton button, const QPointF &localPos, const QPointF &globalPos)
{
    sendMouseMoveEvent(localPos,globalPos);
    wl_pointer *pointer = pointerDevice();
    pointer->button_count--;
    uint32_t time = m_compositor->currentTimeMsecs();
    const struct wl_pointer_grab_interface *interface = pointer->grab->interface;
    interface->button(pointer->grab, time, toWaylandButton(button), 0);
}

void InputDevice::sendMouseMoveEvent(const QPointF &localPos, const QPointF &globalPos)
{
    Q_UNUSED(globalPos);
    uint32_t time = m_compositor->currentTimeMsecs();
    wl_pointer *pointer = pointerDevice();
    const struct wl_pointer_grab_interface *interface = pointer->grab->interface;
    pointer->x = wl_fixed_from_double(globalPos.x());
    pointer->y = wl_fixed_from_double(globalPos.y());
    interface->motion(pointer->grab,
                      time,
                      wl_fixed_from_double(localPos.x()), wl_fixed_from_double(localPos.y()));
}

void InputDevice::sendMouseMoveEvent(Surface *surface, const QPointF &localPos, const QPointF &globalPos)
{
    setMouseFocus(surface,localPos,globalPos);
    sendMouseMoveEvent(localPos,globalPos);
}

void InputDevice::sendMouseWheelEvent(Qt::Orientation orientation, int delta)
{
    wl_pointer *pointer = pointerDevice();
    struct wl_resource *resource = pointer->focus_resource;
    if (!resource)
        return;
    uint32_t time = m_compositor->currentTimeMsecs();
    uint32_t axis = orientation == Qt::Horizontal ? WL_POINTER_AXIS_HORIZONTAL_SCROLL
                                                  : WL_POINTER_AXIS_VERTICAL_SCROLL;
    wl_pointer_send_axis(resource, time, axis, delta);
}

void InputDevice::sendKeyPressEvent(uint code)
{
    wl_keyboard *keyboard = keyboardDevice();
    if (keyboard->focus_resource) {
        uint32_t time = m_compositor->currentTimeMsecs();
        uint32_t serial = wl_display_next_serial(m_compositor->wl_display());
        wl_keyboard_send_key(keyboard->focus_resource,
                             serial, time, code - 8, 1);
    }
}

void InputDevice::sendKeyReleaseEvent(uint code)
{
    wl_keyboard *keyboard = keyboardDevice();
    if (keyboard->focus_resource) {
        uint32_t time = m_compositor->currentTimeMsecs();
        uint32_t serial = wl_display_next_serial(m_compositor->wl_display());
        wl_keyboard_send_key(keyboard->focus_resource,
                             serial, time, code - 8, 0);
    }
}

void InputDevice::sendTouchPointEvent(int id, double x, double y, Qt::TouchPointState state)
{
    uint32_t time = m_compositor->currentTimeMsecs();
    uint32_t serial = 0;
    wl_touch *touch = touchDevice();
    wl_resource *resource = touch->focus_resource;
    if (!resource)
        return;
    switch (state) {
    case Qt::TouchPointPressed:
        wl_touch_send_down(resource, serial, time, &touch->focus->resource, id,
                           wl_fixed_from_double(x), wl_fixed_from_double(y));
        break;
    case Qt::TouchPointMoved:
        wl_touch_send_motion(resource, time, id,
                             wl_fixed_from_double(x), wl_fixed_from_double(y));
        break;
    case Qt::TouchPointReleased:
        wl_touch_send_up(resource, serial, time, id);
        break;
    case Qt::TouchPointStationary:
        // stationary points are not sent through wayland, the client must cache them
        break;
    default:
        break;
    }
}

void InputDevice::sendTouchFrameEvent()
{
    wl_touch *touch = touchDevice();
    wl_resource *resource = touch->focus_resource;
    if (resource)
        wl_touch_send_frame(resource);
}

void InputDevice::sendTouchCancelEvent()
{
    wl_touch *touch = touchDevice();
    wl_resource *resource = touch->focus_resource;
    if (resource)
        wl_touch_send_cancel(resource);
}

void InputDevice::sendFullKeyEvent(QKeyEvent *event)
{
    if (!keyboardFocus()) {
        qWarning("Cannot send key event, no keyboard focus, fix the compositor");
        return;
    }

    QtKeyExtensionGlobal *ext = m_compositor->qtkeyExtension();
    if (ext && ext->postQtKeyEvent(event, keyboardFocus()))
        return;

    if (event->type() == QEvent::KeyPress)
        sendKeyPressEvent(event->nativeScanCode());
    else if (event->type() == QEvent::KeyRelease)
        sendKeyReleaseEvent(event->nativeScanCode());
}

void InputDevice::sendFullTouchEvent(QTouchEvent *event)
{
    if (!mouseFocus()) {
        qWarning("Cannot send touch event, no pointer focus, fix the compositor");
        return;
    }

    if (event->type() == QEvent::TouchCancel) {
        sendTouchCancelEvent();
        return;
    }

    TouchExtensionGlobal *ext = m_compositor->touchExtension();
    if (ext && ext->postTouchEvent(event, mouseFocus()))
        return;

    const QList<QTouchEvent::TouchPoint> points = event->touchPoints();
    if (points.isEmpty())
        return;

    const int pointCount = points.count();
    QPointF pos = mouseFocus()->pos();
    for (int i = 0; i < pointCount; ++i) {
        const QTouchEvent::TouchPoint &tp(points.at(i));
        // Convert the local pos in the compositor window to surface-relative.
        QPointF p = tp.pos() - pos;
        sendTouchPointEvent(tp.id(), p.x(), p.y(), tp.state());
    }
    sendTouchFrameEvent();
}

Surface *InputDevice::keyboardFocus() const
{
    return wayland_cast<Surface>(keyboardDevice()->focus);
}

void InputDevice::setKeyboardFocus(Surface *surface)
{
    sendSelectionFocus(surface);
    wl_keyboard_set_focus(keyboardDevice(), surface ? surface->base() : 0);
}

Surface *InputDevice::mouseFocus() const
{
    return wayland_cast<Surface>(pointerDevice()->focus);
}

void InputDevice::setMouseFocus(Surface *surface, const QPointF &globalPos, const QPointF &localPos)
{
    wl_pointer *pointer = pointerDevice();
    pointer->x = wl_fixed_from_double(globalPos.x());
    pointer->y = wl_fixed_from_double(globalPos.y());
    pointer->current = surface ? surface->base() : 0;
    pointer->current_x = wl_fixed_from_double(localPos.x());
    pointer->current_y = wl_fixed_from_double(localPos.y());
    pointer->grab->interface->focus(pointer->grab,
                                    surface ? surface->base() : 0,
                                    wl_fixed_from_double(localPos.x()), wl_fixed_from_double(localPos.y()));

    // We have no separate touch focus management so make it match the pointer focus always.
    // No wl_touch_set_focus() is available so set it manually.
    wl_touch *touch = touchDevice();
    touch->focus = surface ? surface->base() : 0;
    touch->focus_resource = Compositor::resourceForSurface(&touch->resource_list, surface);
}

void InputDevice::cleanupDataDeviceForClient(struct wl_client *client, bool destroyDev)
{
    for (int i = 0; i < m_data_devices.size(); i++) {
        struct wl_resource *data_device_resource =
                m_data_devices.at(i)->dataDeviceResource();
        if (data_device_resource->client == client) {
            if (destroyDev)
                delete m_data_devices.at(i);
            m_data_devices.removeAt(i);
            break;
        }
    }
}

void InputDevice::clientRequestedDataDevice(DataDeviceManager *data_device_manager, struct wl_client *client, uint32_t id)
{
    cleanupDataDeviceForClient(client, false);
    DataDevice *dataDevice = new DataDevice(data_device_manager,client,id);
    m_data_devices.append(dataDevice);
}

void InputDevice::sendSelectionFocus(Surface *surface)
{
    if (!surface)
        return;
    DataDevice *device = dataDevice(surface->base()->resource.client);
    if (device) {
        device->sendSelectionFocus();
    }
}

Compositor *InputDevice::compositor() const
{
    return m_compositor;
}

WaylandInputDevice *InputDevice::handle() const
{
    return m_handle;
}

uint32_t InputDevice::toWaylandButton(Qt::MouseButton button)
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

DataDevice *InputDevice::dataDevice(struct wl_client *client) const
{
    for (int i = 0; i < m_data_devices.size();i++) {
        if (m_data_devices.at(i)->dataDeviceResource()->client == client) {
            return m_data_devices.at(i);
        }
    }
    return 0;
}

const struct wl_pointer_interface InputDevice::pointer_interface = {
    InputDevice::pointer_attach
};

void InputDevice::pointer_attach(struct wl_client *client,
                                 struct wl_resource *device_resource,
                                 uint32_t serial,
                                 struct wl_resource *buffer_resource, int32_t x, int32_t y)
{
    Q_UNUSED(client);
    Q_UNUSED(serial);

    wl_pointer *pointer = reinterpret_cast<wl_pointer *>(device_resource->data);
    InputDevice *inputDevice = wayland_cast<InputDevice>(pointer->seat);
    wl_buffer *buffer = reinterpret_cast<wl_buffer *>(buffer_resource);

    if (wl_buffer_is_shm(buffer)) {
        int stride = wl_shm_buffer_get_stride(buffer);
        uint format = wl_shm_buffer_get_format(buffer);
        (void) format;
        void *data = wl_shm_buffer_get_data(buffer);
        const uchar *char_data = static_cast<const uchar *>(data);
        if (char_data) {
            QImage *img = new QImage(char_data, buffer->width, buffer->height, stride, QImage::Format_ARGB32_Premultiplied);
            inputDevice->m_compositor->waylandCompositor()->changeCursor(*img, x, y);
            delete currentCursor;
            currentCursor = img;
        }
    }
}


}
