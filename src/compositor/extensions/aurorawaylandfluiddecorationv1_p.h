// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QLoggingCategory>

#include <LiriAuroraCompositor/WaylandFluidDecorationV1>
#include <LiriAuroraCompositor/private/aurora-server-fluid-decoration-unstable-v1.h>

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

Q_DECLARE_LOGGING_CATEGORY(gLcWaylandFluidDecorationV1)

class LIRIAURORACOMPOSITOR_EXPORT WaylandFluidDecorationManagerV1Private
        : public PrivateServer::zfluid_decoration_manager_v1
{
    Q_DECLARE_PUBLIC(WaylandFluidDecorationManagerV1)
public:
    WaylandFluidDecorationManagerV1Private(WaylandFluidDecorationManagerV1 *self);

    bool initialized = false;
    QVector<WaylandFluidDecorationV1 *> decorations;

protected:
    WaylandFluidDecorationManagerV1 *q_ptr;

    void zfluid_decoration_manager_v1_create(Resource *resource, uint32_t id, struct ::wl_resource *surfaceResource) override;
    void zfluid_decoration_manager_v1_destroy(Resource *resource) override;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandFluidDecorationV1Private
        : public PrivateServer::zfluid_decoration_v1
{
    Q_DECLARE_PUBLIC(WaylandFluidDecorationV1)
public:
    WaylandFluidDecorationV1Private(WaylandFluidDecorationV1 *self,
                                    WaylandFluidDecorationManagerV1 *_manager,
                                    WaylandSurface *_surface,
                                    wl_client *client,
                                    quint32 id, quint32 version);

    WaylandFluidDecorationManagerV1 *manager = nullptr;
    WaylandSurface *surface = nullptr;
    QColor fgColor = Qt::transparent;
    QColor bgColor = Qt::transparent;

protected:
    WaylandFluidDecorationV1 *q_ptr;

    void zfluid_decoration_v1_destroy_resource(Resource *resource) override;
    void zfluid_decoration_v1_set_foreground(Resource *resource,
                                             uint32_t r, uint32_t g, uint32_t b, uint32_t a) override;
    void zfluid_decoration_v1_set_background(Resource *resource,
                                             uint32_t r, uint32_t g, uint32_t b, uint32_t a) override;
    void zfluid_decoration_v1_destroy(Resource *resource) override;
};

} // namespace Compositor

} // namespace Aurora

