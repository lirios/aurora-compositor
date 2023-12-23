// SPDX-FileCopyrightText: 2021 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMutexLocker>

#include <LiriAuroraCompositor/WaylandWlrLayerShellV1>
#include <LiriAuroraCompositor/WaylandSurfaceLayout>

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

class LIRIAURORACOMPOSITOR_EXPORT WaylandSurfaceLayoutPrivate
{
    Q_DECLARE_PUBLIC(WaylandSurfaceLayout)
public:
    WaylandSurfaceLayoutPrivate(WaylandSurfaceLayout *self);

    void applyExclusive(WaylandWlrLayerSurfaceV1 *layerSurface, QRectF *availableGeometry);
    void arrangeLayer(WaylandWlrLayerShellV1::Layer layer,
                      bool exclusive, QRectF *availableGeometry);
    void layoutItems();

    QMutex mutex;
    WaylandOutput *output = nullptr;

protected:
    WaylandSurfaceLayout *q_ptr = nullptr;
};

} // namespace Compositor

} // namespace Aurora

