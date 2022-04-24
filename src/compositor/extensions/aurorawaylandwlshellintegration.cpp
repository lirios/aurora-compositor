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

#include "aurorawaylandwlshellintegration_p.h"

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandWlShellSurface>
#include <LiriAuroraCompositor/WaylandQuickShellSurfaceItem>
#include <LiriAuroraCompositor/WaylandSeat>

namespace Aurora {

namespace Compositor {

namespace Internal {

WlShellIntegration::WlShellIntegration(WaylandQuickShellSurfaceItem *item)
    : WaylandQuickShellIntegration(item)
    , m_item(item)
    , m_shellSurface(qobject_cast<WaylandWlShellSurface *>(item->shellSurface()))
{
    m_item->setSurface(m_shellSurface->surface());
    connect(m_shellSurface.data(), &WaylandWlShellSurface::startMove, this, &WlShellIntegration::handleStartMove);
    connect(m_shellSurface.data(), &WaylandWlShellSurface::startResize, this, &WlShellIntegration::handleStartResize);
    connect(m_shellSurface->surface(), &WaylandSurface::redraw, this, &WlShellIntegration::handleRedraw);
    connect(m_shellSurface->surface(), &WaylandSurface::offsetForNextFrame, this, &WlShellIntegration::adjustOffsetForNextFrame);
    connect(m_shellSurface->surface(), &WaylandSurface::hasContentChanged, this, &WlShellIntegration::handleSurfaceHasContentChanged);
    connect(m_shellSurface.data(), &WaylandWlShellSurface::setDefaultToplevel, this, &WlShellIntegration::handleSetDefaultTopLevel);
    connect(m_shellSurface.data(), &WaylandWlShellSurface::setTransient, this, &WlShellIntegration::handleSetTransient);
    connect(m_shellSurface.data(), &WaylandWlShellSurface::setMaximized, this, &WlShellIntegration::handleSetMaximized);
    connect(m_shellSurface.data(), &WaylandWlShellSurface::setFullScreen, this, &WlShellIntegration::handleSetFullScreen);
    connect(m_shellSurface.data(), &WaylandWlShellSurface::setPopup, this, &WlShellIntegration::handleSetPopup);
    connect(m_shellSurface.data(), &WaylandWlShellSurface::destroyed, this, &WlShellIntegration::handleShellSurfaceDestroyed);
}

WlShellIntegration::~WlShellIntegration()
{
    m_item->setSurface(nullptr);
}

void WlShellIntegration::handleStartMove(WaylandSeat *seat)
{
    grabberState = GrabberState::Move;
    moveState.seat = seat;
    moveState.initialized = false;
}

void WlShellIntegration::handleStartResize(WaylandSeat *seat, WaylandWlShellSurface::ResizeEdge edges)
{
    grabberState = GrabberState::Resize;
    resizeState.seat = seat;
    resizeState.resizeEdges = edges;
    resizeState.initialSize = m_shellSurface->surface()->destinationSize();
    resizeState.initialized = false;
}

void WlShellIntegration::handleSetDefaultTopLevel()
{
    // Take focus if the policy allows
    if (m_shellSurface->shell()->focusPolicy() == WaylandShell::AutomaticFocus)
        m_item->takeFocus();

    // In order to restore the window state, the client calls setDefaultToplevel()
    // so we need to unset the flags here but we save the previous state and move
    // to the initial position when redrawing
    nextState = State::Windowed;

    // Any handlers for making maximized or fullscreen state track the size of
    // the designated output, are unneeded now that we're going to windowed
    // state.
    nonwindowedState.output = nullptr;
    disconnect(nonwindowedState.sizeChangedConnection);
}

void WlShellIntegration::handleSetTransient(WaylandSurface *parentSurface, const QPoint &relativeToParent, bool inactive)
{
    Q_UNUSED(parentSurface);
    Q_UNUSED(relativeToParent);

    // Take focus if the policy allows and it's not inactive
    if (m_shellSurface->shell()->focusPolicy() == WaylandShell::AutomaticFocus && !inactive)
        m_item->takeFocus();
}

void WlShellIntegration::handleSetMaximized(WaylandOutput *output)
{
    if (!m_item->view()->isPrimary())
        return;

    if (currentState == State::Maximized)
        return;

    WaylandOutput *designatedOutput = output ? output : m_item->view()->output();
    if (!designatedOutput)
        return;

    if (currentState == State::Windowed)
        normalPosition = m_item->moveItem()->position();

    nextState = State::Maximized;
    finalPosition = designatedOutput->position() + designatedOutput->availableGeometry().topLeft();

    // Any prior output-resize handlers are irrelevant at this point
    disconnect(nonwindowedState.sizeChangedConnection);
    nonwindowedState.output = designatedOutput;
    nonwindowedState.sizeChangedConnection = connect(designatedOutput, &WaylandOutput::availableGeometryChanged, this, &WlShellIntegration::handleMaximizedSizeChanged);
    handleMaximizedSizeChanged();
}

void WlShellIntegration::handleMaximizedSizeChanged()
{
    if (!m_shellSurface)
        return;

    if (nextState == State::Maximized) {
        WaylandOutput *designatedOutput = nonwindowedState.output;
        auto scaleFactor = designatedOutput->scaleFactor();
        m_shellSurface->sendConfigure(designatedOutput->availableGeometry().size() / scaleFactor, WaylandWlShellSurface::NoneEdge);
    }
}

void WlShellIntegration::handleSetFullScreen(WaylandWlShellSurface::FullScreenMethod method, uint framerate, WaylandOutput *output)
{
    Q_UNUSED(method);
    Q_UNUSED(framerate);

    if (!m_item->view()->isPrimary())
        return;

    if (currentState == State::FullScreen)
        return;

    WaylandOutput *designatedOutput = output ? output : m_item->view()->output();
    if (!designatedOutput)
        return;

    if (currentState == State::Windowed)
        normalPosition = m_item->moveItem()->position();

    nextState = State::FullScreen;
    finalPosition = designatedOutput->position();

    // Any prior output-resize handlers are irrelevant at this point
    disconnect(nonwindowedState.sizeChangedConnection);
    nonwindowedState.output = designatedOutput;
    nonwindowedState.sizeChangedConnection = connect(designatedOutput, &WaylandOutput::geometryChanged, this, &WlShellIntegration::handleFullScreenSizeChanged);
    handleFullScreenSizeChanged();
}

void WlShellIntegration::handleFullScreenSizeChanged()
{
    if (!m_shellSurface)
        return;

    if (nextState == State::FullScreen) {
        WaylandOutput *designatedOutput = nonwindowedState.output;
        m_shellSurface->sendConfigure(designatedOutput->geometry().size(), WaylandWlShellSurface::NoneEdge);
    }
}

void WlShellIntegration::handleSetPopup(WaylandSeat *seat, WaylandSurface *parent, const QPoint &relativeToParent)
{
    Q_UNUSED(seat);

    // Find the parent item on the same output
    WaylandQuickShellSurfaceItem *parentItem = nullptr;
    const auto views = parent->views();
    for (WaylandView *view : views) {
        if (view->output() == m_item->view()->output()) {
            WaylandQuickShellSurfaceItem *item = qobject_cast<WaylandQuickShellSurfaceItem*>(view->renderObject());
            if (item) {
                parentItem = item;
                break;
            }
        }
    }

    if (parentItem) {
        // Clear all the transforms for this ShellSurfaceItem. They are not
        // applicable when the item becomes a child to a surface that has its
        // own transforms. Otherwise the transforms would be applied twice.
        QQmlListProperty<QQuickTransform> t = m_item->transform();
        t.clear(&t);
        m_item->setRotation(0);
        m_item->setScale(1.0);
        m_item->setPosition(m_item->mapFromSurface(relativeToParent));
        m_item->setParentItem(parentItem);
    }

    isPopup = true;
    auto shell = m_shellSurface->shell();
    WaylandQuickShellEventFilter::startFilter(m_shellSurface->surface()->client(), [shell]() {
        shell->closeAllPopups();
    });

    QObject::connect(m_shellSurface->surface(), &WaylandSurface::hasContentChanged,
                     this, &WlShellIntegration::handleSurfaceHasContentChanged);
}

void WlShellIntegration::handlePopupClosed()
{
    handlePopupRemoved();
    if (m_shellSurface)
        QObject::disconnect(m_shellSurface->surface(), &WaylandSurface::hasContentChanged,
                            this, &WlShellIntegration::handleSurfaceHasContentChanged);
}

void WlShellIntegration::handlePopupRemoved()
{
    if (!m_shellSurface || m_shellSurface->shell()->mappedPopups().isEmpty())
        WaylandQuickShellEventFilter::cancelFilter();
    isPopup = false;
}

qreal WlShellIntegration::devicePixelRatio() const
{
    return m_item->window() ? m_item->window()->devicePixelRatio() : 1;
}

void WlShellIntegration::handleShellSurfaceDestroyed()
{
    if (isPopup)
        handlePopupRemoved();

    // Disarm any handlers that might fire and attempt to use the now-stale pointer
    nonwindowedState.output = nullptr;
    disconnect(nonwindowedState.sizeChangedConnection);

    m_shellSurface = nullptr;
}

void WlShellIntegration::handleSurfaceHasContentChanged()
{
    if (m_shellSurface && m_shellSurface->surface()->destinationSize().isEmpty()
            && m_shellSurface->windowType() == Qt::WindowType::Popup) {
        handlePopupClosed();
    }
}

void WlShellIntegration::handleRedraw()
{
    if (currentState == nextState)
        return;

    m_item->moveItem()->setPosition(nextState == State::Windowed ? normalPosition : finalPosition);
    currentState = nextState;
}

void WlShellIntegration::adjustOffsetForNextFrame(const QPointF &offset)
{
    if (!m_item->view()->isPrimary())
        return;

    QQuickItem *moveItem = m_item->moveItem();
    moveItem->setPosition(moveItem->position() + m_item->mapFromSurface(offset));
}

bool WlShellIntegration::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        return filterMouseMoveEvent(mouseEvent);
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        return filterMouseReleaseEvent(mouseEvent);
    }
    return WaylandQuickShellIntegration::eventFilter(object, event);
}

bool WlShellIntegration::filterMouseMoveEvent(QMouseEvent *event)
{
    if (grabberState == GrabberState::Resize) {
        Q_ASSERT(resizeState.seat == m_item->compositor()->seatFor(event));
        if (!resizeState.initialized) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            resizeState.initialMousePos = event->scenePosition();
#else
            resizeState.initialMousePos = event->windowPos();
#endif
            resizeState.initialized = true;
            return true;
        }
        float scaleFactor = m_item->view()->output()->scaleFactor();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QPointF delta = (event->scenePosition() - resizeState.initialMousePos) / scaleFactor * devicePixelRatio();
#else
        QPointF delta = (event->windowPos() - resizeState.initialMousePos) / scaleFactor * devicePixelRatio();
#endif
        QSize newSize = m_shellSurface->sizeForResize(resizeState.initialSize, delta, resizeState.resizeEdges);
        m_shellSurface->sendConfigure(newSize, resizeState.resizeEdges);
    } else if (grabberState == GrabberState::Move) {
        Q_ASSERT(moveState.seat == m_item->compositor()->seatFor(event));
        QQuickItem *moveItem = m_item->moveItem();
        if (!moveState.initialized) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            moveState.initialOffset = moveItem->mapFromItem(nullptr, event->scenePosition());
#else
            moveState.initialOffset = moveItem->mapFromItem(nullptr, event->windowPos());
#endif
            moveState.initialized = true;
            return true;
        }
        if (!moveItem->parentItem())
            return true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QPointF parentPos = moveItem->parentItem()->mapFromItem(nullptr, event->scenePosition());
#else
        QPointF parentPos = moveItem->parentItem()->mapFromItem(nullptr, event->windowPos());
#endif
        moveItem->setPosition(parentPos - moveState.initialOffset);
    }
    return false;
}

bool WlShellIntegration::filterMouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (grabberState != GrabberState::Default) {
        grabberState = GrabberState::Default;
        return true;
    }
    return false;
}

}

} // namespace Compositor

} // namespace Aurora
