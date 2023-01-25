// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandivisurfaceintegration_p.h"

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandIviSurface>
#include <LiriAuroraCompositor/WaylandQuickShellSurfaceItem>
#include <LiriAuroraCompositor/WaylandSeat>

namespace Aurora {

namespace Compositor {

namespace Internal {

IviSurfaceIntegration::IviSurfaceIntegration(WaylandQuickShellSurfaceItem *item)
    : WaylandQuickShellIntegration(item)
    , m_item(item)
    , m_shellSurface(qobject_cast<WaylandIviSurface *>(item->shellSurface()))
{
    m_item->setSurface(m_shellSurface->surface());
    connect(m_shellSurface, &WaylandIviSurface::destroyed, this, &IviSurfaceIntegration::handleIviSurfaceDestroyed);
}

IviSurfaceIntegration::~IviSurfaceIntegration()
{
    m_item->setSurface(nullptr);
}

void IviSurfaceIntegration::handleIviSurfaceDestroyed()
{
    m_shellSurface = nullptr;
}

}

} // namespace Compositor

} // namespace Aurora
