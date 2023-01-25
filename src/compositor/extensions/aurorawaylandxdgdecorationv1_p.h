// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDXDGDECORATIONV1_P_H
#define AURORA_COMPOSITOR_WAYLANDXDGDECORATIONV1_P_H

#include "aurorawaylandxdgdecorationv1.h"

#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-xdg-decoration-unstable-v1.h>

#include <LiriAuroraCompositor/WaylandXdgToplevel>

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

class WaylandXdgToplevel;

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgDecorationManagerV1Private
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::zxdg_decoration_manager_v1
{
    Q_DECLARE_PUBLIC(WaylandXdgDecorationManagerV1)
public:
    using DecorationMode = WaylandXdgToplevel::DecorationMode;
    explicit WaylandXdgDecorationManagerV1Private(WaylandXdgDecorationManagerV1 *self);

protected:
    void zxdg_decoration_manager_v1_get_toplevel_decoration(Resource *resource, uint id, ::wl_resource *toplevelResource) override;

private:
    DecorationMode m_preferredMode = DecorationMode::ClientSideDecoration;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgToplevelDecorationV1
        : public PrivateServer::zxdg_toplevel_decoration_v1
{
public:
    using DecorationMode = WaylandXdgToplevel::DecorationMode;
    explicit WaylandXdgToplevelDecorationV1(WaylandXdgToplevel *toplevel,
                                             WaylandXdgDecorationManagerV1 *manager,
                                             wl_client *client, int id);
    ~WaylandXdgToplevelDecorationV1() override;

    DecorationMode configuredMode() const { return DecorationMode(m_configuredMode); }
    void sendConfigure(DecorationMode mode);

protected:
    void zxdg_toplevel_decoration_v1_destroy_resource(Resource *resource) override;
    void zxdg_toplevel_decoration_v1_destroy(Resource *resource) override;
    void zxdg_toplevel_decoration_v1_set_mode(Resource *resource, uint32_t mode) override;
    void zxdg_toplevel_decoration_v1_unset_mode(Resource *resource) override;

private:
    void handleClientPreferredModeChanged();

    WaylandXdgToplevel *m_toplevel = nullptr;
    WaylandXdgDecorationManagerV1 *m_manager = nullptr;
    uint m_configuredMode = 0;
    uint m_clientPreferredMode = 0;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDXDGDECORATIONV1_P_H
