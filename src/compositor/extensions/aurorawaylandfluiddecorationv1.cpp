// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aurorawaylandcompositor.h"
#include "aurorawaylandfluiddecorationv1_p.h"

namespace Aurora {

namespace Compositor {

Q_LOGGING_CATEGORY(gLcWaylandFluidDecorationV1, "aurora.compositor.fluiddecorationv1")

/*
 * WaylandFluidDecorationManagerV1Private
 */

WaylandFluidDecorationManagerV1Private::WaylandFluidDecorationManagerV1Private(WaylandFluidDecorationManagerV1 *self)
    : PrivateServer::zfluid_decoration_manager_v1()
    , q_ptr(self)
{
}

void WaylandFluidDecorationManagerV1Private::zfluid_decoration_manager_v1_create(Resource *resource, uint32_t id, wl_resource *surfaceResource)
{
    Q_Q(WaylandFluidDecorationManagerV1);

    auto surface = WaylandSurface::fromResource(surfaceResource);
    if (!surface) {
        qCWarning(gLcWaylandFluidDecorationV1) << "Couldn't find surface";
        return;
    }

    auto search = [surface](WaylandFluidDecorationV1 *decoration) { return decoration->surface() == surface; };
    if (std::find_if(decorations.begin(), decorations.end(), search) != decorations.end()) {
        qCWarning(gLcWaylandFluidDecorationV1) << "Decoration object already exist for surface";
        wl_resource_post_error(resource->handle, error_already_exists,
                               "zfluid_decoration_v1 already exist for surface");
        return;
    }

    auto decoration = new WaylandFluidDecorationV1(q, surface, resource->client(), id, resource->version());
    decorations.append(decoration);
    Q_EMIT q->decorationCreated(decoration);
}

void WaylandFluidDecorationManagerV1Private::zfluid_decoration_manager_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

/*
 * WaylandFluidDecorationManagerV1
 */

WaylandFluidDecorationManagerV1::WaylandFluidDecorationManagerV1()
    : WaylandCompositorExtensionTemplate<WaylandFluidDecorationManagerV1>()
    , d_ptr(new WaylandFluidDecorationManagerV1Private(this))
{
}

WaylandFluidDecorationManagerV1::WaylandFluidDecorationManagerV1(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate<WaylandFluidDecorationManagerV1>(compositor)
    , d_ptr(new WaylandFluidDecorationManagerV1Private(this))
{
}

WaylandFluidDecorationManagerV1::~WaylandFluidDecorationManagerV1()
{
}

void WaylandFluidDecorationManagerV1::initialize()
{
    Q_D(WaylandFluidDecorationManagerV1);

    if (d->initialized) {
        qCWarning(gLcWaylandFluidDecorationV1) << "Cannot initialize FluidDecorationManagerV1 twice!";
        return;
    }

    d->initialized = true;

    WaylandCompositorExtensionTemplate::initialize();
    auto *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qCWarning(gLcWaylandFluidDecorationV1) << "Failed to find WaylandCompositor when initializing FluidDecorationManagerV1";
        return;
    }
    d->init(compositor->display(), WaylandFluidDecorationManagerV1Private::interfaceVersion());
}

void WaylandFluidDecorationManagerV1::unregisterDecoration(WaylandFluidDecorationV1 *decoration)
{
    Q_D(WaylandFluidDecorationManagerV1);
    d->decorations.removeOne(decoration);
}

const wl_interface *WaylandFluidDecorationManagerV1::interface()
{
    return WaylandFluidDecorationManagerV1Private::interface();
}

QByteArray WaylandFluidDecorationManagerV1::interfaceName()
{
    return WaylandFluidDecorationManagerV1Private::interfaceName();
}

/*
 * WaylandFluidDecorationV1Private
 */

WaylandFluidDecorationV1Private::WaylandFluidDecorationV1Private(WaylandFluidDecorationV1 *self,
                                                                 WaylandFluidDecorationManagerV1 *_manager,
                                                                 WaylandSurface *_surface,
                                                                 wl_client *client,
                                                                 quint32 id, quint32 version)
    : PrivateServer::zfluid_decoration_v1()
    , manager(_manager)
    , surface(_surface)
    , q_ptr(self)
{
    init(client, id, qMin<quint32>(version, interfaceVersion()));
}

void WaylandFluidDecorationV1Private::zfluid_decoration_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)

    Q_Q(WaylandFluidDecorationV1);
    manager->unregisterDecoration(q);
    delete this;
}

void WaylandFluidDecorationV1Private::zfluid_decoration_v1_set_foreground(Resource *resource,
                                                                          uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    Q_UNUSED(resource)

    Q_Q(WaylandFluidDecorationV1);

    QColor color(qRgba(r, g, b, a));
    if (fgColor == color)
        return;
    fgColor = color;
    Q_EMIT q->foregroundColorChanged(fgColor);
}

void WaylandFluidDecorationV1Private::zfluid_decoration_v1_set_background(Resource *resource,
                                                                          uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    Q_UNUSED(resource)

    Q_Q(WaylandFluidDecorationV1);

    QColor color(qRgba(r, g, b, a));
    if (bgColor == color)
        return;
    bgColor = color;
    Q_EMIT q->backgroundColorChanged(bgColor);
}

void WaylandFluidDecorationV1Private::zfluid_decoration_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

/*
 * WaylandFluidDecorationV1
 */

WaylandFluidDecorationV1::WaylandFluidDecorationV1(WaylandFluidDecorationManagerV1 *manager, WaylandSurface *surface,
                                                   wl_client *client,
                                                   quint32 id, quint32 version)
    : QObject()
    , d_ptr(new WaylandFluidDecorationV1Private(this, manager, surface, client, id, version))
{
}

WaylandFluidDecorationV1::~WaylandFluidDecorationV1()
{
}

WaylandSurface *WaylandFluidDecorationV1::surface() const
{
    Q_D(const WaylandFluidDecorationV1);
    return d->surface;
}

wl_resource *WaylandFluidDecorationV1::surfaceResource() const
{
    Q_D(const WaylandFluidDecorationV1);
    return d->surface->resource();
}

QColor WaylandFluidDecorationV1::foregroundColor() const
{
    Q_D(const WaylandFluidDecorationV1);
    return d->fgColor;
}

QColor WaylandFluidDecorationV1::backgroundColor() const
{
    Q_D(const WaylandFluidDecorationV1);
    return d->bgColor;
}

} // namespace Compositor

} // namespace Aurora
