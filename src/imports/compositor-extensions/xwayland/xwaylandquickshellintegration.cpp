/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2018 Pier Luigi Fiorini
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:QTLGPL$
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General
 * Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or (at your option) the GNU General
 * Public license version 3 or any later version approved by the KDE Free
 * Qt Foundation. The licenses are as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 and LICENSE.GPLv3
 * included in the packaging of this file. Please review the following
 * information to ensure the GNU General Public License requirements will
 * be met: https://www.gnu.org/licenses/gpl-2.0.html and
 * https://www.gnu.org/licenses/gpl-3.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <LiriAuroraCompositor/WaylandOutput>

#include "xwaylandquickshellintegration.h"
#include "xwaylandquickshellsurfaceitem.h"

namespace Aurora {

namespace Compositor {

XWaylandQuickShellIntegration::XWaylandQuickShellIntegration(XWaylandQuickShellSurfaceItem *item)
    : m_item(item)
    , m_shellSurface(item->shellSurface())
    , grabberState(GrabberState::Default)
{
    m_item->setSurface(m_shellSurface->surface());

    connect(m_shellSurface, &XWaylandShellSurface::startMove,
            this, &XWaylandQuickShellIntegration::handleStartMove);
    connect(m_shellSurface, &XWaylandShellSurface::startResize,
            this, &XWaylandQuickShellIntegration::handleStartResize);
    connect(m_shellSurface, &XWaylandShellSurface::mapped,
            this, &XWaylandQuickShellIntegration::handleMapped);
    connect(m_shellSurface, &XWaylandShellSurface::activatedChanged,
            this, &XWaylandQuickShellIntegration::handleActivatedChanged);
}

bool XWaylandQuickShellIntegration::mouseMoveEvent(QMouseEvent *event)
{
    if (grabberState == GrabberState::Resize) {
        if (!resizeState.initialized) {
            resizeState.initialMousePos = event->windowPos();
            resizeState.initialized = true;
            return true;
        }
        float scaleFactor = m_item->view()->output()->scaleFactor();
        QPointF delta = (event->windowPos() - resizeState.initialMousePos) / scaleFactor;
        QSize newSize = m_shellSurface->sizeForResize(resizeState.initialSize, delta, resizeState.resizeEdges);
        m_shellSurface->sendResize(newSize);
    } else if (grabberState == GrabberState::Move) {
        QQuickItem *moveItem = m_item->moveItem();
        if (!moveState.initialized) {
            moveState.initialOffset = moveItem->mapFromItem(nullptr, event->windowPos());
            moveState.initialized = true;
            return true;
        }
        if (!moveItem->parentItem())
            return true;
        QPointF parentPos = moveItem->parentItem()->mapFromItem(nullptr, event->windowPos());
        moveItem->setPosition(parentPos - moveState.initialOffset);
    }

    return false;
}

bool XWaylandQuickShellIntegration::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (grabberState != GrabberState::Default) {
        grabberState = GrabberState::Default;
        m_shellSurface->setMoving(false);
        m_shellSurface->setResizing(false);
        return true;
    }
    return false;
}

void XWaylandQuickShellIntegration::handleStartMove()
{
    grabberState = GrabberState::Move;
    moveState.initialized = false;
    m_shellSurface->setMoving(true);
}

void XWaylandQuickShellIntegration::handleStartResize(XWaylandShellSurface::ResizeEdge edges)
{
    grabberState = GrabberState::Resize;
    resizeState.resizeEdges = edges;
    float scaleFactor = m_item->view()->output()->scaleFactor();
    resizeState.initialSize = m_shellSurface->surface()->destinationSize() / scaleFactor;
    resizeState.initialized = false;
    m_shellSurface->setResizing(true);
}

void XWaylandQuickShellIntegration::handleMapped()
{
    if (m_shellSurface->windowType() == Qt::Popup)
        return;

    if (m_shellSurface->wmWindowType() == XWaylandShellSurface::ToplevelWindow)
        m_item->takeFocus();
}

void XWaylandQuickShellIntegration::handleActivatedChanged()
{
    if (m_shellSurface->isActivated())
        m_item->raise();
}

} // namespace Compositor

} // namespace Aurora

#include "moc_xwaylandquickshellintegration.cpp"
