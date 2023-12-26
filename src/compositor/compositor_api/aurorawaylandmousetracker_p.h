// Copyright (C) 2017 The Qt Company Ltd.
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

#include <QtQuick/private/qquickmousearea_p.h>

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
    QML_NAMED_ELEMENT(WaylandMouseTracker)
    QML_ADDED_IN_VERSION(1, 0)
public:
    WaylandMouseTracker(QQuickItem *parent = nullptr);

    qreal mouseX() const;
    qreal mouseY() const;

    void setWindowSystemCursorEnabled(bool enable);
    bool windowSystemCursorEnabled() const;
    bool hovered() const;

Q_SIGNALS:
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
};

} // namespace Compositor

} // namespace Aurora

