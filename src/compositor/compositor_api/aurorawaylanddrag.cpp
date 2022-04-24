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

#include "aurorawaylanddrag.h"

#include <private/qobject_p.h>

#include "aurorawaylandview.h"
#include <LiriAuroraCompositor/private/aurorawaylandseat_p.h>

#if LIRI_FEATURE_aurora_datadevice
#include "aurorawldatadevice_p.h"
#endif

namespace Aurora {

namespace Compositor {

class WaylandDragPrivate : public QObjectPrivate
{
public:
    WaylandDragPrivate(WaylandSeat *seat)
        : seat(seat)
    {
    }

    Internal::DataDevice *dataDevice()
    {
        return WaylandSeatPrivate::get(seat)->dataDevice();
    }

    const Internal::DataDevice *dataDevice() const
    {
        return WaylandSeatPrivate::get(seat)->dataDevice();
    }

    WaylandSeat *seat = nullptr;
};

WaylandDrag::WaylandDrag(WaylandSeat *seat)
    : QObject(* new WaylandDragPrivate(seat))
{
}

WaylandSurface *WaylandDrag::icon() const
{
    Q_D(const WaylandDrag);

    const Internal::DataDevice *dataDevice = d->dataDevice();
    if (!dataDevice)
        return nullptr;

    return dataDevice->dragIcon();
}

WaylandSurface *WaylandDrag::origin() const
{
    Q_D(const WaylandDrag);
    const Internal::DataDevice *dataDevice = d->dataDevice();
    return dataDevice ? dataDevice->dragOrigin() : nullptr;
}

WaylandSeat *WaylandDrag::seat() const
{
    Q_D(const WaylandDrag);
    return d->seat;
}


bool WaylandDrag::visible() const
{
    Q_D(const WaylandDrag);

    const Internal::DataDevice *dataDevice = d->dataDevice();
    if (!dataDevice)
        return false;

    return dataDevice->dragIcon() != nullptr;
}

void WaylandDrag::dragMove(WaylandSurface *target, const QPointF &pos)
{
    Q_D(WaylandDrag);
    Internal::DataDevice *dataDevice = d->dataDevice();
    if (!dataDevice)
        return;
    dataDevice->dragMove(target, pos);
}
void WaylandDrag::drop()
{
    Q_D(WaylandDrag);
    Internal::DataDevice *dataDevice = d->dataDevice();
    if (!dataDevice)
        return;
    dataDevice->drop();
}

void WaylandDrag::cancelDrag()
{
    Q_D(WaylandDrag);
    Internal::DataDevice *dataDevice = d->dataDevice();
    if (!dataDevice)
        return;
    dataDevice->cancelDrag();
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylanddrag.cpp"
