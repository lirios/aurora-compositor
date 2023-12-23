// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
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

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/WaylandDestroyListener>
#include <LiriAuroraCompositor/WaylandPointer>

#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtCore/QObject>

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>
#include <LiriAuroraCompositor/WaylandView>
#include <LiriAuroraCompositor/WaylandSurface>
#include <LiriAuroraCompositor/WaylandSeat>

#include <stdint.h>

namespace Aurora {

namespace Compositor {

class WaylandView;

class LIRIAURORACOMPOSITOR_EXPORT WaylandPointerPrivate : public PrivateServer::wl_pointer
{
    Q_DECLARE_PUBLIC(WaylandPointer)
public:
    WaylandPointerPrivate(WaylandPointer *pointer, WaylandSeat *seat);

    WaylandCompositor *compositor() const { return seat->compositor(); }

protected:
    void pointer_set_cursor(Resource *resource, uint32_t serial, wl_resource *surface, int32_t hotspot_x, int32_t hotspot_y) override;
    void pointer_release(Resource *resource) override;

private:
    uint sendButton(Qt::MouseButton button, uint32_t state);
    void sendMotion();
    void sendEnter(WaylandSurface *surface);
    void sendLeave();
    void ensureEntered(WaylandSurface *surface);

    WaylandPointer *q_ptr = nullptr;
    WaylandSeat *seat = nullptr;
    WaylandOutput *output = nullptr;
    QPointer<WaylandSurface> enteredSurface;

    QPointF localPosition;
    QPointF spacePosition;

    uint enterSerial = 0;

    int buttonCount = 0;

    WaylandDestroyListener enteredSurfaceDestroyListener;

    static WaylandSurfaceRole s_role;
};

} // namespace Compositor

} // namespace Aurora

