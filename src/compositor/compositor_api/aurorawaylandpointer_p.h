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

#ifndef AURORA_COMPOSITOR_WAYLANDPOINTER_P_H
#define AURORA_COMPOSITOR_WAYLANDPOINTER_P_H

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

#include <LiriAuroraCompositor/qtwaylandcompositorglobal.h>
#include <LiriAuroraCompositor/WaylandDestroyListener>
#include <LiriAuroraCompositor/WaylandPointer>

#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtCore/QObject>
#include <QtCore/private/qobject_p.h>

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>
#include <LiriAuroraCompositor/WaylandView>
#include <LiriAuroraCompositor/WaylandSurface>
#include <LiriAuroraCompositor/WaylandSeat>

#include <stdint.h>

namespace Aurora {

namespace Compositor {

class WaylandView;

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandPointerPrivate : public QObjectPrivate
                                                 , public PrivateServer::wl_pointer
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

#endif // AURORA_COMPOSITOR_WAYLANDPOINTER_P_H
