/****************************************************************************
**
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

#ifndef AURORA_COMPOSITOR_WAYLANDDRAG_H
#define AURORA_COMPOSITOR_WAYLANDDRAG_H

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/aurorawaylandqmlinclude.h>

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    Q_MOC_INCLUDE("aurorawaylandsurface.h")
    QML_NAMED_ELEMENT(WaylandDrag)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("")
#endif
public:
    explicit WaylandDrag(WaylandSeat *seat);
    ~WaylandDrag();

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

private:
    QScopedPointer<WaylandDragPrivate> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDDRAG_H
