// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aurorawaylandcompositor.h"
#include "aurorawaylandoutput.h"
#include "aurorawaylandsurface.h"
#include "aurorawaylandwlrlayershellv1_p.h"
#include "aurorawaylandxdgshell_p.h"
#include "aurorawaylandutils_p.h"

#if LIRI_FEATURE_aurora_compositor_quick
#  include "aurorawaylandwlrlayershellintegration_p.h"
#endif

namespace Aurora {

namespace Compositor {

/*
 * WaylandWlrLayerShellV1Private
 */

WaylandWlrLayerShellV1Private::WaylandWlrLayerShellV1Private(WaylandWlrLayerShellV1 *self)
    : WaylandShellPrivate(self)
{
}

void WaylandWlrLayerShellV1Private::unregisterLayerSurface(WaylandWlrLayerSurfaceV1 *layerSurface)
{
    if (!m_layerSurfaces.removeOne(layerSurface))
        qCWarning(gLcAuroraCompositorWlrLayerShellV1,
                  "Unexpected state. Can't find registered layer surface.");
}

void WaylandWlrLayerShellV1Private::closeAllLayerSurfaces()
{
    for (auto *surface : qAsConst(m_layerSurfaces))
        surface->close();
}

void WaylandWlrLayerShellV1Private::zwlr_layer_shell_v1_get_layer_surface(
        PrivateServer::zwlr_layer_shell_v1::Resource *resource,
        uint32_t id, wl_resource *surfaceRes, wl_resource *outputRes,
        uint32_t layer, const QString &nameSpace)
{
    Q_Q(WaylandWlrLayerShellV1);

    // Check the layer value
    if (layer < layer_background || layer > layer_overlay) {
        qCWarning(gLcAuroraCompositorWlrLayerShellV1, "Invalid layer value %d", layer);
        wl_resource_post_error(resource->handle, error_invalid_layer,
                               "invalid layer %d", layer);
        return;
    }

    auto surface = WaylandSurface::fromResource(surfaceRes);
    if (!surface) {
        qCWarning(gLcAuroraCompositorWlrLayerShellV1, "Resource wl_surface@%d doesn't exist",
                  wl_resource_get_id(surfaceRes));
        return;
    }

    // Set role
    if (!surface->setRole(WaylandWlrLayerSurfaceV1::role(), resource->handle, error_role))
        return;

    // Output from resource
    WaylandOutput *output = nullptr;
    if (outputRes)
        output = WaylandOutput::fromResource(outputRes);

    // Create layer surface
    WaylandResource layerSurfaceResource(
                wl_resource_create(resource->client(), &zwlr_layer_surface_v1_interface,
                                   wl_resource_get_version(resource->handle), id));
    auto *layerSurface = new WaylandWlrLayerSurfaceV1(q, surface, output, static_cast<WaylandWlrLayerShellV1::Layer>(layer), nameSpace, layerSurfaceResource);

    m_layerSurfaces.append(layerSurface);
    emit q->layerSurfaceCreated(layerSurface);
}

/*
 * WaylandWlrLayerShellV1
 */

WaylandWlrLayerShellV1::WaylandWlrLayerShellV1()
    : WaylandShellTemplate<WaylandWlrLayerShellV1>()
    , d_ptr(new WaylandWlrLayerShellV1Private(this))
{
}

WaylandWlrLayerShellV1::WaylandWlrLayerShellV1(WaylandCompositor *compositor)
    : WaylandShellTemplate<WaylandWlrLayerShellV1>(compositor)
    , d_ptr(new WaylandWlrLayerShellV1Private(this))
{
}

WaylandWlrLayerShellV1::~WaylandWlrLayerShellV1()
{
    delete d_ptr;
}

void WaylandWlrLayerShellV1::initialize()
{
    Q_D(WaylandWlrLayerShellV1);

    WaylandShellTemplate::initialize();
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qCWarning(gLcAuroraCompositorWlrLayerShellV1) << "Failed to find WaylandCompositor when initializing WaylandWlrLayerShellV1";
        return;
    }
    d->init(compositor->display(), WaylandWlrLayerShellV1Private::interfaceVersion());
}

void WaylandWlrLayerShellV1::closeAllLayerSurfaces()
{
    Q_D(WaylandWlrLayerShellV1);
    d->closeAllLayerSurfaces();
}

const wl_interface *WaylandWlrLayerShellV1::interface()
{
    return WaylandWlrLayerShellV1Private::interface();
}

QByteArray WaylandWlrLayerShellV1::interfaceName()
{
    return WaylandWlrLayerShellV1Private::interfaceName();
}

/*
 * WaylandWlrLayerSurfaceV1Private
 */

WaylandSurfaceRole WaylandWlrLayerSurfaceV1Private::s_role("zwlr_layer_surface_v1");

WaylandWlrLayerSurfaceV1Private::WaylandWlrLayerSurfaceV1Private(WaylandWlrLayerSurfaceV1 *self)
    : WaylandCompositorExtensionPrivate(self)
{
}

WaylandWlrLayerSurfaceV1Private::~WaylandWlrLayerSurfaceV1Private()
{
}

WaylandWlrLayerSurfaceV1Private::ConfigureEvent WaylandWlrLayerSurfaceV1Private::lastSentConfigure() const
{
    return pendingConfigures.isEmpty() ? lastAckedConfigure : pendingConfigures.last();
}

void WaylandWlrLayerSurfaceV1Private::unmap()
{
    Q_Q(WaylandWlrLayerSurfaceV1);

    pendingConfigures.clear();
    if (mapped) {
        mapped = false;
        Q_EMIT q->mappedChanged();
    }
    if (configured) {
        configured = false;
        Q_EMIT q->configuredChanged();
    }
}

void WaylandWlrLayerSurfaceV1Private::zwlr_layer_surface_v1_set_size(PrivateServer::zwlr_layer_surface_v1::Resource *resource, uint32_t width, uint32_t height)
{
    Q_UNUSED(resource)
    clientPending.desiredSize = QSize(width, height);
}

void WaylandWlrLayerSurfaceV1Private::zwlr_layer_surface_v1_set_anchor(PrivateServer::zwlr_layer_surface_v1::Resource *resource, uint32_t anchor)
{
    Q_UNUSED(resource)

    const uint32_t maxAnchor = anchor_top | anchor_bottom | anchor_left | anchor_right;
    if (anchor > maxAnchor) {
        wl_resource_post_error(resource->handle, error_invalid_anchor,
                               "invalid anchor %d", anchor);
        return;
    }

    clientPending.anchors = static_cast<WaylandWlrLayerSurfaceV1::Anchors>(anchor);
}

void WaylandWlrLayerSurfaceV1Private::zwlr_layer_surface_v1_set_exclusive_zone(PrivateServer::zwlr_layer_surface_v1::Resource *resource, int32_t zone)
{
    Q_UNUSED(resource)
    clientPending.exclusiveZone = zone;
}

void WaylandWlrLayerSurfaceV1Private::zwlr_layer_surface_v1_set_margin(PrivateServer::zwlr_layer_surface_v1::Resource *resource, int32_t top, int32_t right, int32_t bottom, int32_t left)
{
    Q_UNUSED(resource)
    clientPending.margins = QMargins(left, top, right, bottom);
}

void WaylandWlrLayerSurfaceV1Private::zwlr_layer_surface_v1_set_keyboard_interactivity(PrivateServer::zwlr_layer_surface_v1::Resource *resource, uint32_t keyboard_interactivity)
{
    Q_UNUSED(resource)
    clientPending.keyboardInteractivity = static_cast<WaylandWlrLayerSurfaceV1::KeyboardInteractivity>(keyboard_interactivity);
}

void WaylandWlrLayerSurfaceV1Private::zwlr_layer_surface_v1_get_popup(PrivateServer::zwlr_layer_surface_v1::Resource *resource, wl_resource *popup)
{
    Q_Q(WaylandWlrLayerSurfaceV1);

    auto *xdgPopup = WaylandXdgPopup::fromResource(popup);
    if (xdgPopup) {
        auto *xdgPopupPrivate = WaylandXdgPopupPrivate::get(xdgPopup);
        xdgPopupPrivate->setParentSurface(surface);
        emit q->xdgPopupParentChanged(xdgPopup);
    }
}

void WaylandWlrLayerSurfaceV1Private::zwlr_layer_surface_v1_ack_configure(PrivateServer::zwlr_layer_surface_v1::Resource *resource, uint32_t serial)
{
    Q_Q(WaylandWlrLayerSurfaceV1);

    ConfigureEvent config;
    bool found = false;

    Q_FOREVER {
        if (pendingConfigures.isEmpty()) {
            qCWarning(gLcAuroraCompositorWlrLayerShellV1, "Received an unexpected ack_configure!");
            return;
        }

        config = pendingConfigures.takeFirst();
        if (config.serial == serial) {
            found = true;
            break;
        }
    }

    if (!found) {
        wl_resource_post_error(resource->handle, error_invalid_surface_state,
                               "wrong configure serial: %u", serial);
        return;
    }

    lastAckedConfigure = config;

    if (!configured) {
        configured = true;
        Q_EMIT q->configuredChanged();
    }
}

void WaylandWlrLayerSurfaceV1Private::zwlr_layer_surface_v1_set_layer(Resource *resource, uint32_t layer)
{
    Q_UNUSED(resource)

    if (layer > PrivateServer::zwlr_layer_shell_v1::layer_overlay) {
        wl_resource_post_error(resource->handle, PrivateServer::zwlr_layer_shell_v1::error_invalid_layer,
                               "invalid layer %d", layer);
        return;
    }

    clientPending.layer = static_cast<WaylandWlrLayerShellV1::Layer>(layer);
}

/*
 * WaylandWlrLayerSurfaceV1
 */

WaylandWlrLayerSurfaceV1::WaylandWlrLayerSurfaceV1(WaylandWlrLayerShellV1 *shell,
                                                   WaylandSurface *surface,
                                                   WaylandOutput *output,
                                                   WaylandWlrLayerShellV1::Layer layer,
                                                   const QString &nameSpace,
                                                   const WaylandResource &resource)
    : WaylandShellSurfaceTemplate<WaylandWlrLayerSurfaceV1>()
    , d_ptr(new WaylandWlrLayerSurfaceV1Private(this))
{
    Q_D(WaylandWlrLayerSurfaceV1);
    initialize(shell, surface, output, layer, nameSpace, resource);

    connect(surface, &WaylandSurface::redraw, this, [this, d] {
        if (d->closed)
            return;

        if (d->surface->hasContent() && !d->configured) {
            qCWarning(gLcAuroraCompositorWlrLayerShellV1, "layer_surface@%u has not yet been configured",
                      wl_resource_get_id(d->resource()->handle));
            wl_resource_post_error(d->resource()->handle,
                                   PrivateServer::zwlr_layer_shell_v1::error_already_constructed,
                                   "layer_surface@%u has never been configured",
                                   wl_resource_get_id(d->resource()->handle));
            return;
        }

        // Set double-buffered properties
        bool hasChanged = false;
        if (d->current.layer != d->clientPending.layer) {
            d->current.layer = d->clientPending.layer;
            Q_EMIT layerChanged();
            hasChanged = true;
        }
        if (d->current.desiredSize != d->clientPending.desiredSize) {
            d->current.desiredSize = d->clientPending.desiredSize;
            Q_EMIT sizeChanged();
            hasChanged = true;
        }
        if (d->current.anchors != d->clientPending.anchors) {
            d->current.anchors = d->clientPending.anchors;
            Q_EMIT anchorsChanged();
            hasChanged = true;
        }
        if (d->current.exclusiveZone != d->clientPending.exclusiveZone) {
            d->current.exclusiveZone = d->clientPending.exclusiveZone;
            Q_EMIT exclusiveZoneChanged();
            hasChanged = true;
        }
        if (d->current.margins != d->clientPending.margins) {
            d->current.margins = d->clientPending.margins;
            Q_EMIT leftMarginChanged();
            Q_EMIT rightMarginChanged();
            Q_EMIT topMarginChanged();
            Q_EMIT bottomMarginChanged();
            hasChanged = true;
        }
        if (d->current.keyboardInteractivity != d->clientPending.keyboardInteractivity) {
            d->current.keyboardInteractivity = d->clientPending.keyboardInteractivity;
            Q_EMIT keyboardInteractivityChanged();
            hasChanged = true;
        }
        if (hasChanged)
            Q_EMIT changed();

        if (!d->added)
            d->added = true;

        if (d->configured && d->surface->hasContent() && !d->mapped) {
            d->mapped = true;
            Q_EMIT mappedChanged();
        }

        if (d->configured && !d->surface->hasContent() && d->mapped)
            d->unmap();
    });
}

WaylandWlrLayerSurfaceV1::~WaylandWlrLayerSurfaceV1()
{
    Q_D(WaylandWlrLayerSurfaceV1);

    if (d->shell)
        WaylandWlrLayerShellV1Private::get(d->shell)->unregisterLayerSurface(this);
}

void WaylandWlrLayerSurfaceV1::initialize(WaylandWlrLayerShellV1 *shell,
                                          WaylandSurface *surface,
                                          WaylandOutput *output,
                                          WaylandWlrLayerShellV1::Layer layer,
                                          const QString &nameSpace,
                                          const WaylandResource &resource)
{
    Q_D(WaylandWlrLayerSurfaceV1);

    d->shell = shell;
    d->surface = surface;
    d->output = output;
    d->current.layer = layer;
    d->clientPending.layer = layer;
    d->nameSpace = nameSpace;

    d->init(resource.resource());
    setExtensionContainer(surface);
    emit surfaceChanged();
    emit shellChanged();
    WaylandCompositorExtension::initialize();
}

WaylandWlrLayerShellV1 *WaylandWlrLayerSurfaceV1::shell() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->shell;
}

void WaylandWlrLayerSurfaceV1::initialize()
{
    WaylandCompositorExtension::initialize();
}

WaylandSurface *WaylandWlrLayerSurfaceV1::surface() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->surface;
}

Aurora::Compositor::WaylandOutput *WaylandWlrLayerSurfaceV1::output() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->output;
}

WaylandWlrLayerShellV1::Layer WaylandWlrLayerSurfaceV1::layer() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->current.layer;
}

QString WaylandWlrLayerSurfaceV1::nameSpace() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->nameSpace;
}

QSize WaylandWlrLayerSurfaceV1::size() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->current.desiredSize;
}

quint32 WaylandWlrLayerSurfaceV1::width() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->current.desiredSize.width();
}

quint32 WaylandWlrLayerSurfaceV1::height() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->current.desiredSize.height();
}

WaylandWlrLayerSurfaceV1::Anchors WaylandWlrLayerSurfaceV1::anchors() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->current.anchors;
}

int WaylandWlrLayerSurfaceV1::exclusiveZone() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->current.exclusiveZone;
}

QMargins WaylandWlrLayerSurfaceV1::margins() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->current.margins;
}

qint32 WaylandWlrLayerSurfaceV1::leftMargin() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->current.margins.left();
}

qint32 WaylandWlrLayerSurfaceV1::topMargin() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->current.margins.top();
}

qint32 WaylandWlrLayerSurfaceV1::rightMargin() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->current.margins.right();
}

qint32 WaylandWlrLayerSurfaceV1::bottomMargin() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->current.margins.bottom();
}

WaylandWlrLayerSurfaceV1::KeyboardInteractivity WaylandWlrLayerSurfaceV1::keyboardInteractivity() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->current.keyboardInteractivity;
}

bool WaylandWlrLayerSurfaceV1::isMapped() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->mapped;
}

bool WaylandWlrLayerSurfaceV1::isConfigured() const
{
    Q_D(const WaylandWlrLayerSurfaceV1);
    return d->configured;
}

quint32 WaylandWlrLayerSurfaceV1::sendConfigure(const QSize &size)
{
    Q_D(WaylandWlrLayerSurfaceV1);

    if (!size.isValid()) {
        qCWarning(gLcAuroraCompositorWlrLayerShellV1) << "Can't configure WaylandWlrLayerSurfaceV1 with invalid size:" << size;
        return 0;
    }

    quint32 serial = d->surface->compositor()->nextSerial();
    d->pendingConfigures.append(WaylandWlrLayerSurfaceV1Private::ConfigureEvent(size, serial));
    d->send_configure(serial, size.width(), size.height());
    return serial;
}

quint32 WaylandWlrLayerSurfaceV1::sendConfigure(int width, int height)
{
    return sendConfigure(QSize(width, height));
}

void WaylandWlrLayerSurfaceV1::close()
{
    Q_D(WaylandWlrLayerSurfaceV1);

    d->closed = true;
    d->unmap();
    d->send_closed();
}

#if LIRI_FEATURE_aurora_compositor_quick
WaylandQuickShellIntegration *WaylandWlrLayerSurfaceV1::createIntegration(WaylandQuickShellSurfaceItem *item)
{
    return new Internal::WlrLayerSurfaceIntegration(item);
}
#endif

/*!
 * Returns the Wayland interface for the WaylandWlrLayerSurfaceV1.
 */
const struct ::wl_interface *WaylandWlrLayerSurfaceV1::interface()
{
    return WaylandWlrLayerSurfaceV1Private::interface();
}

/*!
 * \internal
 */
QByteArray WaylandWlrLayerSurfaceV1::interfaceName()
{
    return WaylandWlrLayerSurfaceV1Private::interfaceName();
}

/*!
 * Returns the WaylandWlrLayerSurfaceV1 corresponding to the \a resource.
 */
WaylandWlrLayerSurfaceV1 *WaylandWlrLayerSurfaceV1::fromResource(struct ::wl_resource *resource)
{
    if (auto p = Internal::fromResource<WaylandWlrLayerSurfaceV1Private *>(resource))
        return p->q_func();
    return nullptr;
}

WaylandSurfaceRole *WaylandWlrLayerSurfaceV1::role()
{
    return &WaylandWlrLayerSurfaceV1Private::s_role;
}

} // namespace Compositor

} // namespace Aurora
