// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/private/aurorawaylandquickshellsurfaceitem_p.h>

#include <LiriAuroraCompositor/WaylandIviSurface>

namespace Aurora {

namespace Compositor {

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

namespace Internal {

class IviSurfaceIntegration : public WaylandQuickShellIntegration
{
    Q_OBJECT
public:
    IviSurfaceIntegration(WaylandQuickShellSurfaceItem *item);
    ~IviSurfaceIntegration() override;

private Q_SLOTS:
    void handleIviSurfaceDestroyed();

private:
    WaylandQuickShellSurfaceItem *m_item = nullptr;
    WaylandIviSurface *m_shellSurface = nullptr;
};

}

} // namespace Compositor

} // namespace Aurora

