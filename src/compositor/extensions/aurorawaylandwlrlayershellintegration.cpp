// SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aurorawaylandxdgshell_p.h"
#include "aurorawaylandwlrlayershellintegration_p.h"
#include "aurorawaylandquickshellsurfaceitem_p.h"

namespace Aurora {

namespace Compositor {

namespace Internal {

WlrLayerSurfaceIntegration::WlrLayerSurfaceIntegration(WaylandQuickShellSurfaceItem *item)
    : WaylandQuickShellIntegration(item)
    , m_layerSurface(qobject_cast<WaylandWlrLayerSurfaceV1 *>(item->shellSurface()))
{
    item->setSurface(m_layerSurface->surface());

    connect(m_layerSurface, &WaylandWlrLayerSurfaceV1::mappedChanged, this, [this, item] {
        if (m_layerSurface->isMapped() && item->parentItem())
            item->parentItem()->polish();
    });
    connect(m_layerSurface, &WaylandWlrLayerSurfaceV1::xdgPopupParentChanged, this, [item](WaylandXdgPopup *popup) {
        if (popup->parentSurface() && item->surface() == popup->parentSurface())
            WaylandQuickShellSurfaceItemPrivate::get(item)->maybeCreateAutoPopup(popup->xdgSurface());
    });
}

} // namespace Internal

} // namespace Compositor

} // namespace Aurora
