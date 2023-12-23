// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/private/aurorawaylandshell_p.h>
#include <LiriAuroraCompositor/private/aurora-server-ivi-application.h>

#include <LiriAuroraCompositor/WaylandIviApplication>

#include <QHash>

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

class LIRIAURORACOMPOSITOR_EXPORT WaylandIviApplicationPrivate
        : public WaylandShellPrivate
        , public PrivateServer::ivi_application
{
    Q_DECLARE_PUBLIC(WaylandIviApplication)

public:
    WaylandIviApplicationPrivate(WaylandIviApplication *self);
    static WaylandIviApplicationPrivate *get(WaylandIviApplication *iviApplication) { return iviApplication->d_func(); }
    void unregisterIviSurface(WaylandIviSurface *iviSurface);

    QHash<uint, WaylandIviSurface*> m_iviSurfaces;

protected:
    void ivi_application_surface_create(Resource *resource, uint32_t ivi_id, wl_resource *surface, uint32_t id) override;
};

} // namespace Compositor

} // namespace Aurora

