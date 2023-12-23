// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

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

#include <LiriAuroraCompositor/WaylandSeat>
#include <LiriAuroraCompositor/private/aurora-server-wayland.h>

namespace Aurora {

namespace Compositor {

namespace Internal {

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

    void setDragFocus(WaylandSurface *focus, const QPointF &localPosition);

    WaylandSurface *dragIcon() const;
    WaylandSurface *dragOrigin() const;

    void dragMove(WaylandSurface *target, const QPointF &pos);
    void drop();
    void cancelDrag();

protected:
    void data_device_start_drag(Resource *resource, struct ::wl_resource *source, struct ::wl_resource *origin, struct ::wl_resource *icon, uint32_t serial) override;
    void data_device_set_selection(Resource *resource, struct ::wl_resource *source, uint32_t serial) override;

private:
    void setDragIcon(WaylandSurface *icon);

    WaylandCompositor *m_compositor = nullptr;
    WaylandSeat *m_seat = nullptr;

    DataSource *m_selectionSource = nullptr;

    struct ::wl_client *m_dragClient = nullptr;
    DataSource *m_dragDataSource = nullptr;

    WaylandSurface *m_dragFocus = nullptr;
    Resource *m_dragFocusResource = nullptr;

    WaylandSurface *m_dragIcon = nullptr;
    WaylandSurface *m_dragOrigin = nullptr;
};

}

} // namespace Compositor

} // namespace Aurora

