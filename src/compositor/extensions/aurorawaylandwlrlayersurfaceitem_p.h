// SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMutexLocker>

#include <LiriAuroraCompositor/WaylandWlrLayerSurfaceItem>

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

namespace Aurora {

namespace Compositor {

class WaylandWlrLayerSurfaceV1;

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrLayerSurfaceItemPrivate
{
    Q_DECLARE_PUBLIC(WaylandWlrLayerSurfaceItem)
public:
    WaylandWlrLayerSurfaceItemPrivate(WaylandWlrLayerSurfaceItem *self);

    void _q_configure();

    WaylandWlrLayerSurfaceV1 *layerSurface = nullptr;

protected:
    WaylandWlrLayerSurfaceItem *q_ptr = nullptr;
};

} // namespace Compositor

} // namespace Aurora

