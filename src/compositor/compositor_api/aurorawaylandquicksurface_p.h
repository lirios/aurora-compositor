// Copyright (C) 2021 The Qt Company Ltd.
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

#include <LiriAuroraCompositor/WaylandQuickSurface>
#include <LiriAuroraCompositor/private/aurorawaylandsurface_p.h>

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickSurfacePrivate : public WaylandSurfacePrivate
{
    Q_DECLARE_PUBLIC(WaylandQuickSurface)
public:
    WaylandQuickSurfacePrivate(WaylandQuickSurface *self)
        : WaylandSurfacePrivate(self)
    {
    }

    bool useTextureAlpha = true;
    bool clientRenderingEnabled = true;
};

} // namespace Compositor

} // namespace Aurora

