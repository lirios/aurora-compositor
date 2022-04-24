/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef AURORA_COMPOSITOR_WAYLANDXDGSHELLINTEGRATION_H
#define AURORA_COMPOSITOR_WAYLANDXDGSHELLINTEGRATION_H

#include <LiriAuroraCompositor/private/aurorawaylandquickshellsurfaceitem_p.h>
#include <LiriAuroraCompositor/WaylandQuickShellSurfaceItem>
#include <LiriAuroraCompositor/WaylandXdgToplevel>

namespace Aurora {

namespace Compositor {

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

class WaylandXdgSurface;

namespace Internal {

class XdgToplevelIntegration : public WaylandQuickShellIntegration
{
    Q_OBJECT
public:
    XdgToplevelIntegration(WaylandQuickShellSurfaceItem *item);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private Q_SLOTS:
    void handleStartMove(Aurora::Compositor::WaylandSeat *seat);
    void handleStartResize(Aurora::Compositor::WaylandSeat *seat, Qt::Edges edges);
    void handleSetMaximized();
    void handleUnsetMaximized();
    void handleMaximizedChanged();
    void handleSetFullscreen();
    void handleUnsetFullscreen();
    void handleFullscreenChanged();
    void handleActivatedChanged();
    void handleSurfaceSizeChanged();
    void handleToplevelDestroyed();
    void handleMaximizedSizeChanged();
    void handleFullscreenSizeChanged();

private:
    WaylandQuickShellSurfaceItem *m_item = nullptr;
    WaylandXdgSurface *m_xdgSurface = nullptr;
    WaylandXdgToplevel *m_toplevel = nullptr;

    enum class GrabberState {
        Default,
        Resize,
        Move
    };
    GrabberState grabberState;

    struct {
        WaylandSeat *seat = nullptr;
        QPointF initialOffset;
        bool initialized;
    } moveState;

    struct {
        WaylandSeat *seat = nullptr;
        Qt::Edges resizeEdges;
        QSizeF initialWindowSize;
        QPointF initialMousePos;
        QPointF initialPosition;
        QSize initialSurfaceSize;
        bool initialized;
    } resizeState;

    struct {
        QSize initialWindowSize;
        QPointF initialPosition;
    } windowedGeometry;

    struct {
        WaylandOutput *output = nullptr;
        QMetaObject::Connection sizeChangedConnection; // Depending on whether maximized or fullscreen,
                                                       // will be hooked to geometry-changed or available-
                                                       // geometry-changed.
    } nonwindowedState;

    bool filterMouseMoveEvent(QMouseEvent *event);
    bool filterMouseReleaseEvent(QMouseEvent *event);
};

class XdgPopupIntegration : public WaylandQuickShellIntegration
{
    Q_OBJECT
public:
    XdgPopupIntegration(WaylandQuickShellSurfaceItem *item);

private Q_SLOTS:
    void handleGeometryChanged();

private:
    WaylandQuickShellSurfaceItem *m_item = nullptr;
    WaylandXdgSurface *m_xdgSurface = nullptr;
    WaylandXdgPopup *m_popup = nullptr;
};

}

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDXDGSHELLINTEGRATION_H
