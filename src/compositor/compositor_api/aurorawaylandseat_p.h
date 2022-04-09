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

#ifndef AURORA_COMPOSITOR_WAYLANDSEAT_P_H
#define AURORA_COMPOSITOR_WAYLANDSEAT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Aurora API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <stdint.h>

#include <LiriAuroraCompositor/private/qtwaylandcompositorglobal_p.h>
#include <LiriAuroraCompositor/aurorawaylandseat.h>

#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtCore/QScopedPointer>
#include <QtCore/private/qobject_p.h>

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>

namespace Aurora {

namespace Compositor {

class QKeyEvent;
class QTouchEvent;
class WaylandSeat;
class WaylandDrag;
class WaylandView;

namespace QtWayland {

class Compositor;
class DataDevice;
class Surface;
class DataDeviceManager;
class Pointer;
class Keyboard;
class Touch;
class InputMethod;

}

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandSeatPrivate : public QObjectPrivate, public PrivateServer::wl_seat
{
public:
    Q_DECLARE_PUBLIC(WaylandSeat)

    WaylandSeatPrivate(WaylandSeat *seat);
    ~WaylandSeatPrivate() override;

    void setCapabilities(WaylandSeat::CapabilityFlags caps);

    static WaylandSeatPrivate *get(WaylandSeat *device) { return device->d_func(); }

#if QT_CONFIG(wayland_datadevice)
    void clientRequestedDataDevice(QtWayland::DataDeviceManager *dndSelection, struct wl_client *client, uint32_t id);
    QtWayland::DataDevice *dataDevice() const { return data_device.data(); }
#endif

protected:
    void seat_bind_resource(wl_seat::Resource *resource) override;

    void seat_get_pointer(wl_seat::Resource *resource,
                          uint32_t id) override;
    void seat_get_keyboard(wl_seat::Resource *resource,
                           uint32_t id) override;
    void seat_get_touch(wl_seat::Resource *resource,
                        uint32_t id) override;

    void seat_destroy_resource(wl_seat::Resource *resource) override;

private:
    bool isInitialized = false;
    WaylandCompositor *compositor = nullptr;
    WaylandView *mouseFocus = nullptr;
    WaylandSurface *keyboardFocus = nullptr;
    WaylandSeat::CapabilityFlags capabilities;

    QScopedPointer<WaylandPointer> pointer;
    QScopedPointer<WaylandKeyboard> keyboard;
    QScopedPointer<WaylandTouch> touch;
#if QT_CONFIG(wayland_datadevice)
    QScopedPointer<QtWayland::DataDevice> data_device;
# if QT_CONFIG(draganddrop)
    QScopedPointer<WaylandDrag> drag_handle;
# endif
#endif
    QScopedPointer<WaylandKeymap> keymap;

};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDSEAT_P_H
