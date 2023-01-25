// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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

    bool filterPointerMoveEvent(const QPointF &scenePosition);
    bool filterMouseMoveEvent(QMouseEvent *event);
    bool filterPointerReleaseEvent();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool filterTouchUpdateEvent(QTouchEvent *event);
#endif
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
