// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include "aurorawaylandviewporter.h"

#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-viewporter.h>

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

class WaylandSurface;

class LIRIAURORACOMPOSITOR_EXPORT WaylandViewporterPrivate
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::wp_viewporter
{
    Q_DECLARE_PUBLIC(WaylandViewporter)
public:
    explicit WaylandViewporterPrivate(WaylandViewporter *self);

    class LIRIAURORACOMPOSITOR_EXPORT Viewport
            : public PrivateServer::wp_viewport
    {
    public:
        explicit Viewport(WaylandSurface *surface, wl_client *client, int id);
        ~Viewport() override;
        void checkCommittedState();

    protected:
        void wp_viewport_destroy_resource(Resource *resource) override;
        void wp_viewport_destroy(Resource *resource) override;
        void wp_viewport_set_source(Resource *resource, wl_fixed_t x, wl_fixed_t y, wl_fixed_t width, wl_fixed_t height) override;
        void wp_viewport_set_destination(Resource *resource, int32_t width, int32_t height) override;

    private:
        QPointer<WaylandSurface> m_surface = nullptr;
    };

protected:
    void wp_viewporter_destroy(Resource *resource) override;
    void wp_viewporter_get_viewport(Resource *resource, uint32_t id, wl_resource *surface) override;
};

} // namespace Compositor

} // namespace Aurora

