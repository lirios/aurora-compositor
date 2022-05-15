// SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aurorawaylandcompositor.h"
#include "aurorawaylandoutput.h"
#include "aurorawaylandsurfacelayout.h"
#include "aurorawaylandwlrlayershellv1_p.h"
#include "aurorawaylandwlrlayersurfaceitem_p.h"

namespace Aurora {

namespace Compositor {

/*
 * WaylandWlrLayerSurfaceItemPrivate
 */

WaylandWlrLayerSurfaceItemPrivate::WaylandWlrLayerSurfaceItemPrivate(WaylandWlrLayerSurfaceItem *self)
    : q_ptr(self)
{
}

void WaylandWlrLayerSurfaceItemPrivate::_q_configure()
{
    Q_Q(WaylandWlrLayerSurfaceItem);

    if (auto *layout = qobject_cast<WaylandSurfaceLayout *>(q->parentItem()))
        layout->update();
}

/*
 * WaylandWlrLayerSurfaceItem
 */

WaylandWlrLayerSurfaceItem::WaylandWlrLayerSurfaceItem(QQuickItem *parent)
    : WaylandQuickShellSurfaceItem(parent)
    , d_ptr(new WaylandWlrLayerSurfaceItemPrivate(this))
{
}

WaylandWlrLayerSurfaceItem::~WaylandWlrLayerSurfaceItem()
{
}

WaylandWlrLayerSurfaceV1 *WaylandWlrLayerSurfaceItem::layerSurface() const
{
    Q_D(const WaylandWlrLayerSurfaceItem);
    return d->layerSurface;
}

void WaylandWlrLayerSurfaceItem::setLayerSurface(WaylandWlrLayerSurfaceV1 *layerSurface)
{
    Q_D(WaylandWlrLayerSurfaceItem);

    if (d->layerSurface == layerSurface)
        return;

    if (layerSurface) {
        if (d->layerSurface)
            disconnect(this, SLOT(_q_configure()));

        setSurface(layerSurface->surface());
        setShellSurface(layerSurface);
        setOutput(layerSurface->output());
        setFocusOnClick(layerSurface->keyboardInteractivity() > WaylandWlrLayerSurfaceV1::NoKeyboardInteractivity);

        d->layerSurface = layerSurface;
        connect(d->layerSurface, SIGNAL(changed()), this, SLOT(_q_configure()));
        emit layerSurfaceChanged(d->layerSurface);
    } else {
        qCWarning(gLcAuroraCompositorWlrLayerShellV1, "Unable to set WaylandWlrLayerSurfaceItem::layerSurface to null");
    }
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandwlrlayersurfaceitem.cpp"
