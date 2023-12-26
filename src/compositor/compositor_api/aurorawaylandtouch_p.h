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
#include <LiriAuroraCompositor/WaylandTouch>
#include <LiriAuroraCompositor/WaylandSeat>
#include <LiriAuroraCompositor/WaylandView>
#include <LiriAuroraCompositor/WaylandCompositor>

#include <QtCore/QPoint>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/private/qobject_p.h>

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandTouchPrivate : public QObjectPrivate, public PrivateServer::wl_touch
{
    Q_DECLARE_PUBLIC(WaylandTouch)
public:
    explicit WaylandTouchPrivate(WaylandTouch *touch, WaylandSeat *seat);

    WaylandCompositor *compositor() const { return seat->compositor(); }

    uint sendDown(WaylandSurface *surface, uint32_t time, int touch_id, const QPointF &position);
    void sendMotion(WaylandClient *client, uint32_t time, int touch_id, const QPointF &position);
    uint sendUp(WaylandClient *client, uint32_t time, int touch_id);

private:
    void touch_release(Resource *resource) override;
    int toSequentialWaylandId(int touchId);

    WaylandSeat *seat = nullptr;
    QVarLengthArray<int, 10> ids;
};

} // namespace Compositor

} // namespace Aurora

