// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
