/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
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

#ifndef AURORA_WAYLAND_TOUCH_P_H
#define AURORA_WAYLAND_TOUCH_P_H

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

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandTouchPrivate : public PrivateServer::wl_touch
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

    WaylandTouch *q_ptr = nullptr;
    WaylandSeat *seat = nullptr;
    QVarLengthArray<int, 10> ids;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_WAYLAND_TOUCH_P_H
