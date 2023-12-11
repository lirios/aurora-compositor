// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/auroraqmlinclude.h>

#include <QtCore/QObject>
#include <QtCore/QPointF>

namespace Aurora {

namespace Compositor {

class WaylandDragPrivate;
class WaylandSurface;
class WaylandSeat;

class LIRIAURORACOMPOSITOR_EXPORT WaylandDrag : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandDrag)

    Q_PROPERTY(Aurora::Compositor::WaylandSurface *icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(bool visible READ visible NOTIFY iconChanged)
    Q_MOC_INCLUDE("aurorawaylandsurface.h")

    QML_NAMED_ELEMENT(WaylandDrag)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("")
public:
    explicit WaylandDrag(WaylandSeat *seat);

    WaylandSurface *icon() const;
    WaylandSurface *origin() const;
    WaylandSeat *seat() const;
    bool visible() const;

public Q_SLOTS:
    void dragMove(Aurora::Compositor::WaylandSurface *target, const QPointF &pos);
    void drop();
    void cancelDrag();

Q_SIGNALS:
    void iconChanged();
    void dragStarted(); // WaylandSurface *icon????
};

} // namespace Compositor

} // namespace Aurora

