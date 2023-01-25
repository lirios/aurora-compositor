// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDIDLEINHIBITV1_P_H
#define AURORA_COMPOSITOR_WAYLANDIDLEINHIBITV1_P_H

#include <LiriAuroraCompositor/WaylandSurface>
#include <LiriAuroraCompositor/WaylandIdleInhibitManagerV1>
#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-idle-inhibit-unstable-v1.h>

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

class LIRIAURORACOMPOSITOR_EXPORT WaylandIdleInhibitManagerV1Private
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::zwp_idle_inhibit_manager_v1
{
    Q_DECLARE_PUBLIC(WaylandIdleInhibitManagerV1)
public:
    explicit WaylandIdleInhibitManagerV1Private(WaylandIdleInhibitManagerV1 *self);

    class LIRIAURORACOMPOSITOR_EXPORT Inhibitor
            : public PrivateServer::zwp_idle_inhibitor_v1
    {
    public:
        explicit Inhibitor(WaylandSurface *surface, wl_client *client, quint32 id, quint32 version);

    protected:
        void zwp_idle_inhibitor_v1_destroy_resource(Resource *resource) override;
        void zwp_idle_inhibitor_v1_destroy(Resource *resource) override;

    private:
        QPointer<WaylandSurface> m_surface;
    };

    static WaylandIdleInhibitManagerV1Private *get(WaylandIdleInhibitManagerV1 *manager) { return manager ? manager->d_func() : nullptr; }

protected:
    void zwp_idle_inhibit_manager_v1_create_inhibitor(Resource *resource, uint32_t id, wl_resource *surfaceResource) override;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDIDLEINHIBITV1_P_H
