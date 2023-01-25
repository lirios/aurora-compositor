// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDMOUSETRACKER_P_H
#define AURORA_COMPOSITOR_WAYLANDMOUSETRACKER_P_H

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

#include <QQuickItem>

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>

namespace Aurora {

namespace Compositor {

class WaylandMouseTrackerPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandMouseTracker : public QQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandMouseTracker)
    Q_PROPERTY(qreal mouseX READ mouseX NOTIFY mouseXChanged)
    Q_PROPERTY(qreal mouseY READ mouseY NOTIFY mouseYChanged)
    Q_PROPERTY(bool containsMouse READ hovered NOTIFY hoveredChanged)
    Q_PROPERTY(bool windowSystemCursorEnabled READ windowSystemCursorEnabled WRITE setWindowSystemCursorEnabled NOTIFY windowSystemCursorEnabledChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(WaylandMouseTracker)
    QML_ADDED_IN_VERSION(1, 0)
#endif
public:
    WaylandMouseTracker(QQuickItem *parent = nullptr);
    ~WaylandMouseTracker();

    qreal mouseX() const;
    qreal mouseY() const;

    void setWindowSystemCursorEnabled(bool enable);
    bool windowSystemCursorEnabled() const;
    bool hovered() const;

signals:
    void mouseXChanged();
    void mouseYChanged();
    void windowSystemCursorEnabledChanged();
    void hoveredChanged();

protected:
    bool childMouseEventFilter(QQuickItem *item, QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;

private:
    QScopedPointer<WaylandMouseTrackerPrivate> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

#endif  /*QWAYLANDMOUSETRACKER_P_H*/
