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

#ifndef WLDATADEVICE_H
#define WLDATADEVICE_H

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

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>
#include <LiriAuroraCompositor/private/qtwaylandcompositorglobal_p.h>
#include <LiriAuroraCompositor/WaylandSeat>

QT_REQUIRE_CONFIG(wayland_datadevice);

namespace Aurora {

namespace Compositor {

namespace QtWayland {

class Compositor;
class DataSource;
class Seat;
class Surface;

class DataDevice : public PrivateServer::wl_data_device
{
public:
    DataDevice(WaylandSeat *seat);

    void setFocus(WaylandClient *client);
    void sourceDestroyed(DataSource *source);

#if QT_CONFIG(draganddrop)
    void setDragFocus(WaylandSurface *focus, const QPointF &localPosition);

    WaylandSurface *dragIcon() const;
    WaylandSurface *dragOrigin() const;

    void dragMove(WaylandSurface *target, const QPointF &pos);
    void drop();
    void cancelDrag();
#endif

protected:
#if QT_CONFIG(draganddrop)
    void data_device_start_drag(Resource *resource, struct ::wl_resource *source, struct ::wl_resource *origin, struct ::wl_resource *icon, uint32_t serial) override;
#endif
    void data_device_set_selection(Resource *resource, struct ::wl_resource *source, uint32_t serial) override;

private:
#if QT_CONFIG(draganddrop)
    void setDragIcon(WaylandSurface *icon);
#endif

    WaylandCompositor *m_compositor = nullptr;
    WaylandSeat *m_seat = nullptr;

    DataSource *m_selectionSource = nullptr;

#if QT_CONFIG(draganddrop)
    struct ::wl_client *m_dragClient = nullptr;
    DataSource *m_dragDataSource = nullptr;

    WaylandSurface *m_dragFocus = nullptr;
    Resource *m_dragFocusResource = nullptr;

    WaylandSurface *m_dragIcon = nullptr;
    WaylandSurface *m_dragOrigin = nullptr;
#endif
};

}

} // namespace Compositor

} // namespace Aurora

#endif // WLDATADEVICE_H
