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

#ifndef AURORA_COMPOSITOR_WAYLANDWLSHELLINTEGRATION_H
#define AURORA_COMPOSITOR_WAYLANDWLSHELLINTEGRATION_H

#include <LiriAuroraCompositor/private/aurorawaylandquickshellsurfaceitem_p.h>

#include <LiriAuroraCompositor/WaylandWlShellSurface>

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

namespace Internal {

class WlShellIntegration : public WaylandQuickShellIntegration
{
    Q_OBJECT
public:
    WlShellIntegration(WaylandQuickShellSurfaceItem *item);
    ~WlShellIntegration() override;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private Q_SLOTS:
    void handleStartMove(Aurora::Compositor::WaylandSeat *seat);
    void handleStartResize(Aurora::Compositor::WaylandSeat *seat, WaylandWlShellSurface::ResizeEdge edges);
    void handleSetDefaultTopLevel();
    void handleSetTransient(Aurora::Compositor::WaylandSurface *parentSurface, const QPoint &relativeToParent, bool inactive);
    void handleSetMaximized(Aurora::Compositor::WaylandOutput *output);
    void handleSetFullScreen(WaylandWlShellSurface::FullScreenMethod method, uint framerate, Aurora::Compositor::WaylandOutput *output);
    void handleSetPopup(Aurora::Compositor::WaylandSeat *seat, Aurora::Compositor::WaylandSurface *parent, const QPoint &relativeToParent);
    void handleShellSurfaceDestroyed();
    void handleSurfaceHasContentChanged();
    void handleRedraw();
    void adjustOffsetForNextFrame(const QPointF &offset);
    void handleFullScreenSizeChanged();
    void handleMaximizedSizeChanged();

private:
    enum class GrabberState {
        Default,
        Resize,
        Move
    };

    void handlePopupClosed();
    void handlePopupRemoved();
    qreal devicePixelRatio() const;

    WaylandQuickShellSurfaceItem *m_item = nullptr;
    QPointer<WaylandWlShellSurface> m_shellSurface;
    GrabberState grabberState = GrabberState::Default;
    struct {
        WaylandSeat *seat = nullptr;
        QPointF initialOffset;
        bool initialized = false;
    } moveState;
    struct {
        WaylandSeat *seat = nullptr;
        WaylandWlShellSurface::ResizeEdge resizeEdges;
        QSizeF initialSize;
        QPointF initialMousePos;
        bool initialized = false;
    } resizeState;

    bool isPopup = false;

    enum class State {
        Windowed,
        Maximized,
        FullScreen
    };

    State currentState = State::Windowed;
    State nextState = State::Windowed;

    struct {
        WaylandOutput *output = nullptr;
        QMetaObject::Connection sizeChangedConnection; // Depending on whether maximized or fullscreen,
                                                       // will be hooked to geometry-changed or available-
                                                       // geometry-changed.
    } nonwindowedState;

    QPointF normalPosition;
    QPointF finalPosition;

    bool filterMouseMoveEvent(QMouseEvent *event);
    bool filterMouseReleaseEvent(QMouseEvent *event);
};

}

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDWLSHELLINTEGRATION_H
