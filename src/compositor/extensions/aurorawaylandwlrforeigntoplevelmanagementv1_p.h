// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <LiriAuroraCompositor/WaylandClient>
#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandOutput>
#include <LiriAuroraCompositor/WaylandSeat>
#include <LiriAuroraCompositor/WaylandSurface>
#include <LiriAuroraCompositor/WaylandWlrForeignToplevelManagerV1>
#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-wlr-foreign-toplevel-management-unstable-v1.h>

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

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrForeignToplevelManagerV1Private
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::zwlr_foreign_toplevel_manager_v1
{
    Q_DECLARE_PUBLIC(WaylandWlrForeignToplevelManagerV1)
public:
    explicit WaylandWlrForeignToplevelManagerV1Private(WaylandWlrForeignToplevelManagerV1 *self);

    static WaylandWlrForeignToplevelManagerV1Private *get(WaylandWlrForeignToplevelManagerV1 *self) { return self->d_func(); }

    WaylandCompositor *compositor = nullptr;
    QVector<WaylandWlrForeignToplevelHandleV1 *> toplevels;
    QVector<wl_client *> stoppedClients;

protected:
    void zwlr_foreign_toplevel_manager_v1_bind_resource(Resource *resource) override;
    void zwlr_foreign_toplevel_manager_v1_stop(Resource *resource) override;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrForeignToplevelHandleV1Private
        : public PrivateServer::zwlr_foreign_toplevel_handle_v1
{
    Q_DECLARE_PUBLIC(WaylandWlrForeignToplevelHandleV1)
public:
    explicit WaylandWlrForeignToplevelHandleV1Private(WaylandWlrForeignToplevelHandleV1 *self);

    static WaylandWlrForeignToplevelHandleV1Private *get(WaylandWlrForeignToplevelHandleV1 *self) { return self->d_func(); }

    bool initialized = false;
    WaylandCompositor *compositor = nullptr;
    WaylandWlrForeignToplevelManagerV1 *manager = nullptr;
    QVector<state> states;
    QString title, appId;
    WaylandOutput *output = nullptr;
    bool enteredOutput = false;
    WaylandSurface *rectSurface = nullptr;
    QRect rect;
    WaylandWlrForeignToplevelHandleV1 *parentHandle = nullptr;

protected:
    WaylandWlrForeignToplevelHandleV1 *q_ptr;

    void zwlr_foreign_toplevel_handle_v1_set_maximized(Resource *resource) override;
    void zwlr_foreign_toplevel_handle_v1_unset_maximized(Resource *resource) override;
    void zwlr_foreign_toplevel_handle_v1_set_minimized(Resource *resource) override;
    void zwlr_foreign_toplevel_handle_v1_unset_minimized(Resource *resource) override;
    void zwlr_foreign_toplevel_handle_v1_activate(Resource *resource,
                                                  struct ::wl_resource *seatResource) override;
    void zwlr_foreign_toplevel_handle_v1_close(Resource *resource) override;
    void zwlr_foreign_toplevel_handle_v1_set_rectangle(Resource *resource,
                                                       struct ::wl_resource *surfaceResource,
                                                       int32_t x, int32_t y, int32_t width, int32_t height) override;
    void zwlr_foreign_toplevel_handle_v1_destroy(Resource *resource) override;
    void zwlr_foreign_toplevel_handle_v1_set_fullscreen(Resource *resource,
                                                        struct ::wl_resource *outputResource) override;
    void zwlr_foreign_toplevel_handle_v1_unset_fullscreen(Resource *resource) override;
};

} // namespace Compositor

} // namespace Aurora

