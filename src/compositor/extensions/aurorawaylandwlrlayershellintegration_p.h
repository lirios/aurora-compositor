// SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AURORA_COMPOSITOR_WAYLANDWLRLAYERSHELLINTEGRATION_P_H
#define AURORA_COMPOSITOR_WAYLANDWLRLAYERSHELLINTEGRATION_P_H

#include <LiriAuroraCompositor/WaylandQuickShellIntegration>
#include <LiriAuroraCompositor/WaylandQuickShellSurfaceItem>
#include <LiriAuroraCompositor/WaylandWlrLayerShellV1>

namespace Aurora {

namespace Compositor {

namespace Internal {

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

class WlrLayerSurfaceIntegration : public WaylandQuickShellIntegration
{
    Q_OBJECT
public:
    WlrLayerSurfaceIntegration(WaylandQuickShellSurfaceItem *item);

private:
    WaylandWlrLayerSurfaceV1 *m_layerSurface = nullptr;
};

} // namespace Private

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDWLRLAYERSHELLINTEGRATION_P_H
