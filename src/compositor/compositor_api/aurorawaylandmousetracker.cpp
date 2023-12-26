// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandmousetracker_p.h"

#include <QtQuick/private/qquickitem_p.h>

namespace Aurora {

namespace Compositor {

class WaylandMouseTrackerPrivate : public QQuickItemPrivate
{
    Q_DECLARE_PUBLIC(WaylandMouseTracker)
public:
    WaylandMouseTrackerPrivate()
    {
        QImage cursorImage(64,64,QImage::Format_ARGB32);
        cursorImage.fill(Qt::transparent);
        cursorPixmap = QPixmap::fromImage(cursorImage);
    }
    void handleMousePos(const QPointF &mousePos)
    {
        Q_Q(WaylandMouseTracker);
        bool xChanged = mousePos.x() != this->mousePos.x();
        bool yChanged = mousePos.y() != this->mousePos.y();
        if (xChanged || yChanged) {
            this->mousePos = mousePos;
            if (xChanged)
                emit q->mouseXChanged();
            if (yChanged)
                emit q->mouseYChanged();
        }
    }

    void setHovered(bool hovered)
    {
        Q_Q(WaylandMouseTracker);
        if (this->hovered == hovered)
            return;
        this->hovered = hovered;
        emit q->hoveredChanged();
    }

    QPointF mousePos;
    bool windowSystemCursorEnabled = false;
    QPixmap cursorPixmap;
    bool hovered = false;
};

WaylandMouseTracker::WaylandMouseTracker(QQuickItem *parent)
        : QQuickItem(*(new WaylandMouseTrackerPrivate), parent)
{
    Q_D(WaylandMouseTracker);
    setFiltersChildMouseEvents(true);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
#if QT_CONFIG(cursor)
    setCursor(QCursor(d->cursorPixmap));
#endif
}

qreal WaylandMouseTracker::mouseX() const
{
    Q_D(const WaylandMouseTracker);
    return d->mousePos.x();
}
qreal WaylandMouseTracker::mouseY() const
{
    Q_D(const WaylandMouseTracker);
    return d->mousePos.y();
}

#if QT_CONFIG(cursor)
void WaylandMouseTracker::setWindowSystemCursorEnabled(bool enable)
{
    Q_D(WaylandMouseTracker);
    if (d->windowSystemCursorEnabled != enable) {
        d->windowSystemCursorEnabled = enable;
        if (enable) {
            unsetCursor();
        } else {
            setCursor(QCursor(d->cursorPixmap));
        }
        emit windowSystemCursorEnabledChanged();
    }
}

bool WaylandMouseTracker::windowSystemCursorEnabled() const
{
    Q_D(const WaylandMouseTracker);
    return d->windowSystemCursorEnabled;
}
#endif

bool WaylandMouseTracker::hovered() const
{
    Q_D(const WaylandMouseTracker);
    return d->hovered;
}

bool WaylandMouseTracker::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    Q_D(WaylandMouseTracker);
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        d->handleMousePos(mapFromItem(item, mouseEvent->position()));
    } else if (event->type() == QEvent::HoverMove) {
        QHoverEvent *hoverEvent = static_cast<QHoverEvent *>(event);
        d->handleMousePos(mapFromItem(item, hoverEvent->position()));
    }
    return false;
}

void WaylandMouseTracker::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(WaylandMouseTracker);
    QQuickItem::mouseMoveEvent(event);
    d->handleMousePos(event->position());
}

void WaylandMouseTracker::hoverMoveEvent(QHoverEvent *event)
{
    Q_D(WaylandMouseTracker);
    QQuickItem::hoverMoveEvent(event);
    d->handleMousePos(event->position());
}

void WaylandMouseTracker::hoverEnterEvent(QHoverEvent *event)
{
    Q_D(WaylandMouseTracker);
    Q_UNUSED(event);
    d->setHovered(true);
}

void WaylandMouseTracker::hoverLeaveEvent(QHoverEvent *event)
{
    Q_D(WaylandMouseTracker);
    Q_UNUSED(event);
    d->setHovered(false);
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandmousetracker_p.cpp"
