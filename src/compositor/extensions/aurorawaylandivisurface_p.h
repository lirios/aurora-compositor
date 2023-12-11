// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-ivi-application.h>

#include <LiriAuroraCompositor/WaylandIviSurface>

#include <LiriAuroraCompositor/WaylandSurfaceRole>

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

class LIRIAURORACOMPOSITOR_EXPORT WaylandIviSurfacePrivate
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::ivi_surface
{
    Q_DECLARE_PUBLIC(WaylandIviSurface)

public:
    WaylandIviSurfacePrivate();
    static WaylandIviSurfacePrivate *get(WaylandIviSurface *iviSurface) { return iviSurface->d_func(); }

protected:
    void ivi_surface_destroy_resource(Resource *resource) override;
    void ivi_surface_destroy(Resource *resource) override;

private:
    WaylandIviApplication *m_iviApplication = nullptr;
    WaylandSurface *m_surface = nullptr;
    uint m_iviId = UINT_MAX;

    static WaylandSurfaceRole s_role;
};

} // namespace Compositor

} // namespace Aurora

