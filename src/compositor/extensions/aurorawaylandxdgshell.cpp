/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "aurorawaylandxdgshell.h"
#include "aurorawaylandxdgshell_p.h"

#if LIRI_FEATURE_aurora_compositor_quick
#include "aurorawaylandxdgshellintegration_p.h"
#endif
#include <LiriAuroraCompositor/private/aurorawaylandutils_p.h>

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandSeat>
#include <LiriAuroraCompositor/WaylandSurface>
#include <LiriAuroraCompositor/WaylandSurfaceRole>
#include <LiriAuroraCompositor/WaylandResource>

#include <QtCore/QObject>

#include <algorithm>

namespace Aurora {

namespace Compositor {

WaylandXdgShellPrivate::WaylandXdgShellPrivate(WaylandXdgShell *self)
    : WaylandShellPrivate(self)
{
}

void WaylandXdgShellPrivate::ping(PrivateServer::xdg_wm_base::Resource *resource, uint32_t serial)
{
    m_pings.insert(serial);
    send_ping(resource->handle, serial);
}

void WaylandXdgShellPrivate::registerXdgSurface(WaylandXdgSurface *xdgSurface)
{
    m_xdgSurfaces.insert(xdgSurface->surface()->client()->client(), xdgSurface);
}

void WaylandXdgShellPrivate::unregisterXdgSurface(WaylandXdgSurface *xdgSurface)
{
    auto xdgSurfacePrivate = WaylandXdgSurfacePrivate::get(xdgSurface);
    if (!m_xdgSurfaces.remove(xdgSurfacePrivate->resource()->client(), xdgSurface))
        qWarning("%s Unexpected state. Can't find registered xdg surface\n", Q_FUNC_INFO);
}

WaylandXdgSurface *WaylandXdgShellPrivate::xdgSurfaceFromSurface(WaylandSurface *surface)
{
    for (WaylandXdgSurface *xdgSurface : qAsConst(m_xdgSurfaces)) {
        if (surface == xdgSurface->surface())
            return xdgSurface;
    }
    return nullptr;
}

void WaylandXdgShellPrivate::xdg_wm_base_destroy(Resource *resource)
{
    if (!m_xdgSurfaces.values(resource->client()).empty())
        wl_resource_post_error(resource->handle, XDG_WM_BASE_ERROR_DEFUNCT_SURFACES,
                               "xdg_shell was destroyed before children");

    wl_resource_destroy(resource->handle);
}

void WaylandXdgShellPrivate::xdg_wm_base_create_positioner(PrivateServer::xdg_wm_base::Resource *resource, uint32_t id)
{
    WaylandResource positionerResource(wl_resource_create(resource->client(), &xdg_positioner_interface,
                                                           wl_resource_get_version(resource->handle), id));

    new WaylandXdgPositioner(positionerResource);
}

void WaylandXdgShellPrivate::xdg_wm_base_get_xdg_surface(Resource *resource, uint32_t id, wl_resource *surfaceResource)
{
    Q_Q(WaylandXdgShell);
    WaylandSurface *surface = WaylandSurface::fromResource(surfaceResource);

    if (surface->role() != nullptr) {
        wl_resource_post_error(resource->handle, XDG_WM_BASE_ERROR_ROLE,
                               "wl_surface@%d, already has role %s\n",
                               wl_resource_get_id(surface->resource()),
                               surface->role()->name().constData());
        return;
    }

    if (surface->hasContent()) {
        //TODO: According to the spec, this is a client error, but there's no appropriate error code
        qWarning() << "get_xdg_surface requested on a xdg_surface with content";
    }

    WaylandResource xdgSurfaceResource(wl_resource_create(resource->client(), &xdg_surface_interface,
                                                           wl_resource_get_version(resource->handle), id));

    WaylandXdgSurface *xdgSurface = new WaylandXdgSurface(q, surface, xdgSurfaceResource);

    registerXdgSurface(xdgSurface);
    emit q->xdgSurfaceCreated(xdgSurface);
}

void WaylandXdgShellPrivate::xdg_wm_base_pong(Resource *resource, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_Q(WaylandXdgShell);
    if (m_pings.remove(serial))
        emit q->pong(serial);
    else
        qWarning("Received an unexpected pong!");
}

/*!
 * \qmltype XdgShell
 * \instantiates WaylandXdgShell
 * \inqmlmodule Aurora.Compositor.XdgShell
 * \since 5.12
 * \brief Provides an extension for desktop-style user interfaces.
 *
 * The XdgShell extension provides a way to associate a XdgToplevel or XdgPopup
 * with a regular Wayland surface. Using the XdgToplevel interface, the client
 * can request that the surface is resized, moved, and so on.
 *
 * XdgShell corresponds to the Wayland interface, \c xdg_shell.
 *
 * To provide the functionality of the shell extension in a compositor, create
 * an instance of the XdgShell component and add it to the list of extensions
 * supported by the compositor:
 *
 * \qml
 * import Aurora.Compositor.XdgShell
 *
 * WaylandCompositor {
 *     XdgShell {
 *         // ...
 *     }
 * }
 * \endqml
 */

/*!
 * \class WaylandXdgShell
 * \inmodule AuroraCompositor
 * \since 5.12
 * \brief The WaylandXdgShell class is an extension for desktop-style user interfaces.
 *
 * The WaylandXdgShell extension provides a way to associate a WaylandXdgToplevel or
 * WaylandXdgPopup with a regular Wayland surface. Using the WaylandXdgToplevel interface,
 * the client can request that the surface is resized, moved, and so on.
 *
 * WaylandXdgShell corresponds to the Wayland interface, \c xdg_shell.
 */

/*!
 * Constructs a WaylandXdgShell object.
 */
WaylandXdgShell::WaylandXdgShell()
    : WaylandShellTemplate<WaylandXdgShell>()
    , d_ptr(new WaylandXdgShellPrivate(this))
{
}

/*!
 * Constructs a WaylandXdgShell object for the provided \a compositor.
 */
WaylandXdgShell::WaylandXdgShell(WaylandCompositor *compositor)
    : WaylandShellTemplate<WaylandXdgShell>(compositor)
    , d_ptr(new WaylandXdgShellPrivate(this))
{
}

WaylandXdgShell::~WaylandXdgShell()
{
}

/*!
 * Initializes the shell extension.
 */
void WaylandXdgShell::initialize()
{
    Q_D(WaylandXdgShell);
    WaylandShellTemplate::initialize();
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find WaylandCompositor when initializing WaylandXdgShell";
        return;
    }
    d->init(compositor->display(), 1);

    handleSeatChanged(compositor->defaultSeat(), nullptr);

    connect(compositor, &WaylandCompositor::defaultSeatChanged,
            this, &WaylandXdgShell::handleSeatChanged);
}

/*!
 * Returns the Wayland interface for the WaylandXdgShell.
 */
const struct wl_interface *WaylandXdgShell::interface()
{
    return WaylandXdgShellPrivate::interface();
}

QByteArray WaylandXdgShell::interfaceName()
{
    return WaylandXdgShellPrivate::interfaceName();
}

/*!
 * \qmlmethod void AuroraCompositor::XdgShell::ping(WaylandClient client)
 *
 * Sends a ping event to \a client. If the client replies to the event the
 * \l pong signal will be emitted.
 */

/*!
 * Sends a ping event to \a client. If the client replies to the event the
 * \l pong signal will be emitted.
 */
uint WaylandXdgShell::ping(WaylandClient *client)
{
    Q_D(WaylandXdgShell);

    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    Q_ASSERT(compositor);

    uint32_t serial = compositor->nextSerial();

    WaylandXdgShellPrivate::Resource *clientResource = d->resourceMap().value(client->client(), nullptr);
    Q_ASSERT(clientResource);

    d->ping(clientResource, serial);
    return serial;
}

void WaylandXdgShell::handleSeatChanged(WaylandSeat *newSeat, WaylandSeat *oldSeat)
{
    if (oldSeat != nullptr) {
        disconnect(oldSeat, &WaylandSeat::keyboardFocusChanged,
                   this, &WaylandXdgShell::handleFocusChanged);
    }

    if (newSeat != nullptr) {
        connect(newSeat, &WaylandSeat::keyboardFocusChanged,
                this, &WaylandXdgShell::handleFocusChanged);
    }
}

void WaylandXdgShell::handleFocusChanged(WaylandSurface *newSurface, WaylandSurface *oldSurface)
{
    Q_D(WaylandXdgShell);

    WaylandXdgSurface *newXdgSurface = d->xdgSurfaceFromSurface(newSurface);
    WaylandXdgSurface *oldXdgSurface = d->xdgSurfaceFromSurface(oldSurface);

    if (newXdgSurface)
        WaylandXdgSurfacePrivate::get(newXdgSurface)->handleFocusReceived();

    if (oldXdgSurface)
        WaylandXdgSurfacePrivate::get(oldXdgSurface)->handleFocusLost();
}

WaylandXdgSurfacePrivate::WaylandXdgSurfacePrivate(WaylandXdgSurface *self)
    : WaylandCompositorExtensionPrivate(self)
{
}

void WaylandXdgSurfacePrivate::setWindowType(Qt::WindowType windowType)
{
    if (m_windowType == windowType)
        return;

    m_windowType = windowType;

    Q_Q(WaylandXdgSurface);
    emit q->windowTypeChanged();
}

void WaylandXdgSurfacePrivate::handleFocusLost()
{
    if (m_toplevel)
        WaylandXdgToplevelPrivate::get(m_toplevel)->handleFocusLost();
}

void WaylandXdgSurfacePrivate::handleFocusReceived()
{
    if (m_toplevel)
        WaylandXdgToplevelPrivate::get(m_toplevel)->handleFocusReceived();
}

QRect WaylandXdgSurfacePrivate::calculateFallbackWindowGeometry() const
{
    // TODO: The unset window geometry should include subsurfaces as well, so this solution
    // won't work too well on those kinds of clients.
    return QRect(QPoint(), m_surface->destinationSize());
}

void WaylandXdgSurfacePrivate::updateFallbackWindowGeometry()
{
    Q_Q(WaylandXdgSurface);
    if (!m_unsetWindowGeometry)
        return;

    const QRect unsetGeometry = calculateFallbackWindowGeometry();
    if (unsetGeometry == m_windowGeometry)
        return;

    m_windowGeometry = unsetGeometry;
    emit q->windowGeometryChanged();
}

void WaylandXdgSurfacePrivate::xdg_surface_destroy_resource(PrivateServer::xdg_surface::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(WaylandXdgSurface);
    WaylandXdgShellPrivate::get(m_xdgShell)->unregisterXdgSurface(q);
    delete q;
}

void WaylandXdgSurfacePrivate::xdg_surface_destroy(PrivateServer::xdg_surface::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void WaylandXdgSurfacePrivate::xdg_surface_get_toplevel(PrivateServer::xdg_surface::Resource *resource, uint32_t id)
{
    Q_Q(WaylandXdgSurface);

    if (m_toplevel || m_popup) {
        wl_resource_post_error(resource->handle, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED,
                               "xdg_surface already has a role object");
        return;
    }

    if (!m_surface->setRole(WaylandXdgToplevel::role(), resource->handle, XDG_WM_BASE_ERROR_ROLE))
        return;

    WaylandResource topLevelResource(wl_resource_create(resource->client(), &xdg_toplevel_interface,
                                                         wl_resource_get_version(resource->handle), id));

    m_toplevel = new WaylandXdgToplevel(q, topLevelResource);
    emit q->toplevelCreated();
    emit m_xdgShell->toplevelCreated(m_toplevel, q);
}

void WaylandXdgSurfacePrivate::xdg_surface_get_popup(PrivateServer::xdg_surface::Resource *resource, uint32_t id, wl_resource *parentResource, wl_resource *positionerResource)
{
    Q_Q(WaylandXdgSurface);

    if (m_toplevel || m_popup) {
        wl_resource_post_error(resource->handle, XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED,
                               "xdg_surface already has a role object");
        return;
    }

    WaylandXdgSurface *parent = WaylandXdgSurface::fromResource(parentResource);
    if (!parent) {
        wl_resource_post_error(resource->handle, XDG_WM_BASE_ERROR_INVALID_POPUP_PARENT,
                               "xdg_surface.get_popup with invalid popup parent");
        return;
    }

    WaylandXdgPositioner *positioner = WaylandXdgPositioner::fromResource(positionerResource);
    if (!positioner) {
        wl_resource_post_error(resource->handle, XDG_WM_BASE_ERROR_INVALID_POSITIONER,
                               "xdg_surface.get_popup without positioner");
        return;
    }

    if (!positioner->m_data.isComplete()) {
        WaylandXdgPositionerData p = positioner->m_data;
        wl_resource_post_error(resource->handle, XDG_WM_BASE_ERROR_INVALID_POSITIONER,
                               "xdg_surface.get_popup with invalid positioner (size: %dx%d, anchorRect: %dx%d)",
                               p.size.width(), p.size.height(), p.anchorRect.width(), p.anchorRect.height());
        return;
    }

    QRect anchorBounds(QPoint(0, 0), parent->windowGeometry().size());
    if (!anchorBounds.contains(positioner->m_data.anchorRect)) {
        // TODO: this is a protocol error and should ideally be handled like this:
        //wl_resource_post_error(resource->handle, XDG_WM_BASE_ERROR_INVALID_POSITIONER,
        //                       "xdg_positioner anchor rect extends beyound its parent's window geometry");
        //return;
        // However, our own clients currently do this, so we'll settle for a gentle warning instead.
        qCWarning(gLcAuroraCompositor) << "Ignoring client protocol error: xdg_positioner anchor"
                                        << "rect extends beyond its parent's window geometry";
    }

    if (!m_surface->setRole(WaylandXdgPopup::role(), resource->handle, XDG_WM_BASE_ERROR_ROLE))
        return;

    WaylandResource popupResource(wl_resource_create(resource->client(), &xdg_popup_interface,
                                                      wl_resource_get_version(resource->handle), id));

    m_popup = new WaylandXdgPopup(q, parent, positioner, popupResource);
    emit q->popupCreated();
    emit m_xdgShell->popupCreated(m_popup, q);
}

void WaylandXdgSurfacePrivate::xdg_surface_ack_configure(PrivateServer::xdg_surface::Resource *resource, uint32_t serial)
{
    if (m_toplevel) {
        WaylandXdgToplevelPrivate::get(m_toplevel)->handleAckConfigure(serial);
    } else if (m_popup) {
        WaylandXdgPopupPrivate::get(m_popup)->handleAckConfigure(serial);
    } else {
        wl_resource_post_error(resource->handle, XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
                               "ack_configure requested on an unconstructed xdg_surface");
    }
}

void WaylandXdgSurfacePrivate::xdg_surface_set_window_geometry(PrivateServer::xdg_surface::Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_Q(WaylandXdgSurface);

    if (!q->surface()->role()) {
        wl_resource_post_error(resource->handle, XDG_SURFACE_ERROR_NOT_CONSTRUCTED,
                               "set_window_geometry requested on an unconstructed xdg_surface");
        return;
    }

    if (width <= 0 || height <= 0) {
        // The protocol spec says "setting an invalid size will raise an error". But doesn't tell
        // which error to raise, and there's no fitting error in the xdg_surface_error enum.
        // So until this is fixed, just output a warning and return.
        qWarning() << "Invalid (non-positive) dimensions received in set_window_geometry";
        return;
    }

    m_unsetWindowGeometry = false;

    QRect geometry(x, y, width, height);

    if (m_windowGeometry == geometry)
        return;

    m_windowGeometry = geometry;
    emit q->windowGeometryChanged();
}

/*!
 * \qmltype XdgSurface
 * \instantiates WaylandXdgSurface
 * \inqmlmodule Aurora.Compositor.XdgShell
 * \since 5.12
 * \brief XdgSurface provides desktop-style compositor-specific features to an xdg surface.
 *
 * This type is part of the \l{XdgShell} extension and provides a way to
 * extend the functionality of an existing \l{WaylandSurface} with features
 * specific to desktop-style compositors, such as resizing and moving the
 * surface.
 *
 * It corresponds to the Wayland interface \c xdg_surface.
 */

/*!
 * \class WaylandXdgSurface
 * \inmodule AuroraCompositor
 * \since 5.12
 * \brief The WaylandXdgSurface class provides desktop-style compositor-specific features to an xdg surface.
 *
 * This class is part of the WaylandXdgShell extension and provides a way to
 * extend the functionality of an existing WaylandSurface with features
 * specific to desktop-style compositors, such as resizing and moving the
 * surface.
 *
 * It corresponds to the Wayland interface \c xdg_surface.
 */

/*!
 * Constructs a WaylandXdgSurface.
 */
WaylandXdgSurface::WaylandXdgSurface()
    : WaylandShellSurfaceTemplate<WaylandXdgSurface>()
    , d_ptr(new WaylandXdgSurfacePrivate(this))
{
}

/*!
 * Constructs a WaylandXdgSurface for \a surface and initializes it with the
 * given \a xdgShell, \a surface, and resource \a res.
 */
WaylandXdgSurface::WaylandXdgSurface(WaylandXdgShell *xdgShell, WaylandSurface *surface, const WaylandResource &res)
    : WaylandShellSurfaceTemplate<WaylandXdgSurface>()
    , d_ptr(new WaylandXdgSurfacePrivate(this))
{
    initialize(xdgShell, surface, res);
}

WaylandXdgSurface::~WaylandXdgSurface()
{
}

/*!
 * \qmlmethod void AuroraCompositor::XdgSurface::initialize(object xdgShell, object surface, object client, int id)
 *
 * Initializes the XdgSurface, associating it with the given \a xdgShell, \a surface,
 * \a client, and \a id.
 */

/*!
 * Initializes the WaylandXdgSurface, associating it with the given \a xdgShell, \a surface
 * and \a resource.
 */
void WaylandXdgSurface::initialize(WaylandXdgShell *xdgShell, WaylandSurface *surface, const WaylandResource &resource)
{
    Q_D(WaylandXdgSurface);
    d->m_xdgShell = xdgShell;
    d->m_surface = surface;
    d->init(resource.resource());
    setExtensionContainer(surface);
    d->m_windowGeometry = d->calculateFallbackWindowGeometry();
    connect(surface, &WaylandSurface::destinationSizeChanged, this, &WaylandXdgSurface::handleSurfaceSizeChanged);
    connect(surface, &WaylandSurface::bufferScaleChanged, this, &WaylandXdgSurface::handleBufferScaleChanged);
    emit shellChanged();
    emit surfaceChanged();
    WaylandCompositorExtension::initialize();
}

/*!
 * \qmlproperty enum AuroraCompositor::XdgSurface::windowType
 *
 * This property holds the window type of the XdgSurface.
 */
Qt::WindowType WaylandXdgSurface::windowType() const
{
    Q_D(const WaylandXdgSurface);
    return d->m_windowType;
}

/*!
 * \qmlproperty rect AuroraCompositor::XdgSurface::windowGeometry
 *
 * This property holds the window geometry of the WaylandXdgSurface. The window
 * geometry describes the window's visible bounds from the user's perspective.
 * The geometry includes title bars and borders if drawn by the client, but
 * excludes drop shadows. It is meant to be used for aligning and tiling
 * windows.
 */

/*!
 * \property WaylandXdgSurface::windowGeometry
 *
 * This property holds the window geometry of the WaylandXdgSurface. The window
 * geometry describes the window's visible bounds from the user's perspective.
 * The geometry includes title bars and borders if drawn by the client, but
 * excludes drop shadows. It is meant to be used for aligning and tiling
 * windows.
 */
QRect WaylandXdgSurface::windowGeometry() const
{
    Q_D(const WaylandXdgSurface);
    return d->m_windowGeometry;
}

/*!
 * \internal
 */
void WaylandXdgSurface::initialize()
{
    WaylandCompositorExtension::initialize();
}

void WaylandXdgSurface::handleSurfaceSizeChanged()
{
    Q_D(WaylandXdgSurface);
    d->updateFallbackWindowGeometry();
}

void WaylandXdgSurface::handleBufferScaleChanged()
{
    Q_D(WaylandXdgSurface);
    d->updateFallbackWindowGeometry();
}

/*!
 * \qmlproperty XdgShell AuroraCompositor::XdgSurface::shell
 *
 * This property holds the shell associated with this XdgSurface.
 */

/*!
 * \property WaylandXdgSurface::shell
 *
 * This property holds the shell associated with this WaylandXdgSurface.
 */
WaylandXdgShell *WaylandXdgSurface::shell() const
{
    Q_D(const WaylandXdgSurface);
    return d->m_xdgShell;
}

/*!
 * \qmlproperty WaylandSurface AuroraCompositor::XdgSurface::surface
 *
 * This property holds the surface associated with this XdgSurface.
 */

/*!
 * \property WaylandXdgSurface::surface
 *
 * This property holds the surface associated with this WaylandXdgSurface.
 */
WaylandSurface *WaylandXdgSurface::surface() const
{
    Q_D(const WaylandXdgSurface);
    return d->m_surface;
}

/*!
 * \qmlproperty XdgToplevel AuroraCompositor::XdgSurface::toplevel
 *
 * This property holds the properties and methods that are specific to the
 * toplevel XdgSurface.
 *
 * \sa popup, XdgShell::toplevelCreated
 */

/*!
 * \property WaylandXdgSurface::toplevel
 *
 * This property holds the properties and methods that are specific to the
 * toplevel WaylandXdgSurface.
 *
 * \sa WaylandXdgSurface::popup, WaylandXdgShell::toplevelCreated
 */
WaylandXdgToplevel *WaylandXdgSurface::toplevel() const
{
    Q_D(const WaylandXdgSurface);
    return d->m_toplevel;
}

/*!
 * \qmlproperty XdgPopup AuroraCompositor::XdgSurface::popup
 *
 * This property holds the properties and methods that are specific to the
 * popup XdgSurface.
 *
 * \sa toplevel, XdgShell::popupCreated
 */

/*!
 * \property WaylandXdgSurface::popup
 *
 * This property holds the properties and methods that are specific to the
 * popup WaylandXdgSurface.
 *
 * \sa WaylandXdgSurface::toplevel, WaylandXdgShell::popupCreated
 */
WaylandXdgPopup *WaylandXdgSurface::popup() const
{
    Q_D(const WaylandXdgSurface);
    return d->m_popup;
}

/*!
 * Returns the Wayland interface for the WaylandXdgSurface.
 */
const wl_interface *WaylandXdgSurface::interface()
{
    return WaylandXdgSurfacePrivate::interface();
}

/*!
 * \internal
 */
QByteArray WaylandXdgSurface::interfaceName()
{
    return WaylandXdgSurfacePrivate::interfaceName();
}

/*!
 * Returns the WaylandXdgSurface corresponding to the \a resource.
 */
WaylandXdgSurface *WaylandXdgSurface::fromResource(wl_resource *resource)
{
    if (auto p = Internal::fromResource<WaylandXdgSurfacePrivate *>(resource))
        return p->q_func();
    return nullptr;
}

#if LIRI_FEATURE_aurora_compositor_quick
WaylandQuickShellIntegration *WaylandXdgSurface::createIntegration(WaylandQuickShellSurfaceItem *item)
{
    Q_D(const WaylandXdgSurface);

    if (d->m_toplevel)
        return new Internal::XdgToplevelIntegration(item);

    if (d->m_popup)
        return new Internal::XdgPopupIntegration(item);

    return nullptr;
}
#endif

/*!
 * \qmltype XdgToplevel
 * \instantiates WaylandXdgToplevel
 * \inqmlmodule Aurora.Compositor.XdgShell
 * \since 5.12
 * \brief XdgToplevel represents the toplevel window specific parts of an xdg surface.
 *
 * This type is part of the \l{XdgShell} extension and provides a way to
 * extend the functionality of an XdgSurface with features
 * specific to desktop-style windows.
 *
 * It corresponds to the Wayland interface \c xdg_toplevel.
 */

/*!
 * \class WaylandXdgToplevel
 * \inmodule AuroraCompositor
 * \since 5.12
 * \brief The WaylandXdgToplevel class represents the toplevel window specific parts of an xdg surface.
 *
 * This class is part of the WaylandXdgShell extension and provides a way to
 * extend the functionality of an WaylandXdgSurface with features
 * specific to desktop-style windows.
 *
 * It corresponds to the Wayland interface \c xdg_toplevel.
 */

/*!
 * Constructs a WaylandXdgToplevel for the given \a xdgSurface and \a resource.
 */
WaylandXdgToplevel::WaylandXdgToplevel(WaylandXdgSurface *xdgSurface, WaylandResource &resource)
    : QObject()
    , d_ptr(new WaylandXdgToplevelPrivate(this, xdgSurface, resource))
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<WaylandXdgToplevel::State> states;
#else
    QVector<WaylandXdgToplevel::State> states;
#endif
    sendConfigure({0, 0}, states);
}

WaylandXdgToplevel::~WaylandXdgToplevel()
{
    Q_D(WaylandXdgToplevel);
    // Usually, the decoration is destroyed by the client (according to the protocol),
    // but if the client misbehaves, or is shut down, we need to clean up here.
    if (Q_UNLIKELY(d->m_decoration))
        wl_resource_destroy(d->m_decoration->resource()->handle);
    Q_ASSERT(!d->m_decoration);
}

/*!
 * \qmlproperty XdgSurface AuroraCompositor::XdgToplevel::xdgSurface
 *
 * This property holds the XdgSurface for this XdgToplevel.
 */

/*!
 * \property WaylandXdgToplevel::xdgSurface
 *
 * This property holds the WaylandXdgSurface for this WaylandXdgToplevel.
 */
WaylandXdgSurface *WaylandXdgToplevel::xdgSurface() const
{
    Q_D(const WaylandXdgToplevel);
    return d->m_xdgSurface;
}

/*!
 * \qmlproperty XdgToplevel AuroraCompositor::XdgToplevel::parentToplevel
 *
 * This property holds the XdgToplevel parent of this XdgToplevel.
 */

/*!
 * \property WaylandXdgToplevel::parentToplevel
 *
 * This property holds the XdgToplevel parent of this XdgToplevel.
 *
 */
WaylandXdgToplevel *WaylandXdgToplevel::parentToplevel() const
{
    Q_D(const WaylandXdgToplevel);
    return d->m_parentToplevel;
}

/*!
 * \qmlproperty string AuroraCompositor::XdgToplevel::title
 *
 * This property holds the title of the XdgToplevel.
 */

/*!
 * \property WaylandXdgToplevel::title
 *
 * This property holds the title of the WaylandXdgToplevel.
 */
QString WaylandXdgToplevel::title() const
{
    Q_D(const WaylandXdgToplevel);
    return d->m_title;
}

/*!
 * \qmlproperty string AuroraCompositor::XdgToplevel::appId
 *
 * This property holds the app id of the XdgToplevel.
 */

/*!
 * \property WaylandXdgToplevel::appId
 *
 * This property holds the app id of the WaylandXdgToplevel.
 */
QString WaylandXdgToplevel::appId() const
{
    Q_D(const WaylandXdgToplevel);
    return d->m_appId;
}

/*!
 * \qmlproperty size AuroraCompositor::XdgToplevel::maxSize
 *
 * This property holds the maximum size of the XdgToplevel as requested by the client.
 *
 * The compositor is free to ignore this value and request a larger size.
 */

/*!
 * \property WaylandXdgToplevel::maxSize
 *
 * This property holds the maximum size of the WaylandXdgToplevel.
 *
 * The compositor is free to ignore this value and request a larger size.
 */
QSize WaylandXdgToplevel::maxSize() const
{
    Q_D(const WaylandXdgToplevel);
    return d->m_maxSize;
}

/*!
 * \qmlproperty size AuroraCompositor::XdgToplevel::minSize
 *
 * This property holds the minimum size of the XdgToplevel as requested by the client.
 *
 * The compositor is free to ignore this value and request a smaller size.
 */

/*!
 * \property WaylandXdgToplevel::minSize
 *
 * This property holds the minimum size of the WaylandXdgToplevel.
 *
 * The compositor is free to ignore this value and request a smaller size.
 */
QSize WaylandXdgToplevel::minSize() const
{
    Q_D(const WaylandXdgToplevel);
    return d->m_minSize;
}

/*!
 * \property WaylandXdgToplevel::states
 *
 * This property holds the last states the client acknowledged for this WaylandToplevel.
 */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QList<WaylandXdgToplevel::State> WaylandXdgToplevel::states() const
#else
QVector<WaylandXdgToplevel::State> WaylandXdgToplevel::states() const
#endif
{
    Q_D(const WaylandXdgToplevel);
    return d->m_lastAckedConfigure.states;
}

/*!
 * \qmlproperty bool AuroraCompositor::XdgToplevel::maximized
 *
 * This property holds whether the client has acknowledged that it should be maximized.
 */

/*!
 * \property WaylandXdgToplevel::maximized
 *
 * This property holds whether the client has acknowledged that it should be maximized.
 */
bool WaylandXdgToplevel::maximized() const
{
    Q_D(const WaylandXdgToplevel);
    return d->m_lastAckedConfigure.states.contains(WaylandXdgToplevel::State::MaximizedState);
}

/*!
 * \qmlproperty bool AuroraCompositor::XdgToplevel::fullscreen
 *
 * This property holds whether the client has acknowledged that it should be fullscreen.
 */

/*!
 * \property WaylandXdgToplevel::fullscreen
 *
 * This property holds whether the client has acknowledged that it should be fullscreen.
 */
bool WaylandXdgToplevel::fullscreen() const
{
    Q_D(const WaylandXdgToplevel);
    return d->m_lastAckedConfigure.states.contains(WaylandXdgToplevel::State::FullscreenState);
}

/*!
 * \qmlproperty bool AuroraCompositor::XdgToplevel::resizing
 *
 * This property holds whether the client has acknowledged that it is being resized.
 */

/*!
 * \property WaylandXdgToplevel::resizing
 *
 * This property holds whether the client has acknowledged that it is being resized.
 */
bool WaylandXdgToplevel::resizing() const
{
    Q_D(const WaylandXdgToplevel);
    return d->m_lastAckedConfigure.states.contains(WaylandXdgToplevel::State::ResizingState);
}

/*!
 * \qmlproperty bool AuroraCompositor::XdgToplevel::activated
 *
 * This property holds whether toplevel is drawing itself as having input focus.
 */

/*!
 * \property WaylandXdgToplevel::activated
 *
 * This property holds whether toplevel is drawing itself as having input focus.
 */
bool WaylandXdgToplevel::activated() const
{
    Q_D(const WaylandXdgToplevel);
    return d->m_lastAckedConfigure.states.contains(WaylandXdgToplevel::State::ActivatedState);
}

/*!
 * \enum WaylandXdgToplevel::DecorationMode
 *
 * This enum type is used to specify the window decoration mode for toplevel windows.
 *
 * \value ServerSideDecoration The compositor should draw window decorations.
 * \value ClientSideDecoration The client should draw window decorations.
 */

/*!
 * \qmlproperty enumeration AuroraCompositor::XdgToplevel::decorationMode
 *
 * This property holds the current window decoration mode for this toplevel.
 *
 * The possible values are:
 * \value XdgToplevel.ServerSideDecoration The compositor should draw window decorations.
 * \value XdgToplevel.ClientSideDecoration The client should draw window decorations.
 *
 * \sa XdgDecorationManagerV1
 */

/*!
 * \property WaylandXdgToplevel::decorationMode
 *
 * This property holds the current window decoration mode for this toplevel.
 *
 * \sa WaylandXdgDecorationManagerV1
 */
WaylandXdgToplevel::DecorationMode WaylandXdgToplevel::decorationMode() const
{
    Q_D(const WaylandXdgToplevel);
    return d->m_decoration ? d->m_decoration->configuredMode() : DecorationMode::ClientSideDecoration;
}

/*!
 * \qmlmethod size AuroraCompositor::XdgToplevel::sizeForResize(size size, point delta, uint edges)
 *
 * Convenience for computing the new size given the current \a size, a \a delta, and
 * the \a edges active in the drag.
 */

/*!
 * Convenience for computing the new size given the current \a size, a \a delta, and
 * the \a edges active in the drag.
 */
QSize WaylandXdgToplevel::sizeForResize(const QSizeF &size, const QPointF &delta, Qt::Edges edges) const
{
    qreal width = size.width();
    qreal height = size.height();
    if (edges & Qt::LeftEdge)
        width -= delta.x();
    else if (edges & Qt::RightEdge)
        width += delta.x();

    if (edges & Qt::TopEdge)
        height -= delta.y();
    else if (edges & Qt::BottomEdge)
        height += delta.y();

    QSize newSize = QSize(width, height)
            .expandedTo(minSize())
            .expandedTo({1, 1}); // We don't want to send a size of (0,0) as that means that the client decides

    if (maxSize().isValid())
        newSize = newSize.boundedTo(maxSize());

    return newSize;
}

/*!
 * Sends a configure event to the client. Parameter \a size contains the pixel size
 * of the surface. A size of zero means the client is free to decide the size.
 * Known \a states are enumerated in WaylandXdgToplevel::State.
 */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
uint WaylandXdgToplevel::sendConfigure(const QSize &size, const QList<WaylandXdgToplevel::State> &states)
#else
uint WaylandXdgToplevel::sendConfigure(const QSize &size, const QVector<WaylandXdgToplevel::State> &states)
#endif
{
    if (!size.isValid()) {
        qWarning() << "Can't configure xdg_toplevel with an invalid size" << size;
        return 0;
    }
    Q_D(WaylandXdgToplevel);
    auto statesBytes = QByteArray::fromRawData(reinterpret_cast<const char *>(states.data()),
                                               states.size() * static_cast<int>(sizeof(State)));
    uint32_t serial = d->m_xdgSurface->surface()->compositor()->nextSerial();
    d->m_pendingConfigures.append(WaylandXdgToplevelPrivate::ConfigureEvent{states, size, serial});
    d->send_configure(size.width(), size.height(), statesBytes);
    WaylandXdgSurfacePrivate::get(d->m_xdgSurface)->send_configure(serial);
    return serial;
}

/*!
 * \qmlmethod int AuroraCompositor::XdgToplevel::sendConfigure(size size, list<int> states)
 *
 * Sends a configure event to the client. \a size contains the pixel size of the surface.
 * A size of zero means the client is free to decide the size.
 * Known \a states are enumerated in XdgToplevel::State.
 */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
uint WaylandXdgToplevel::sendConfigure(const QSize &size, const QList<int> &states)
#else
uint WaylandXdgToplevel::sendConfigure(const QSize &size, const QVector<int> &states)
#endif
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<State> s;
#else
    QVector<State> s;
#endif
    for (auto state : states)
        s << State(state);
    return sendConfigure(size, s);
}

/*!
 * \qmlmethod void AuroraCompositor::XdgToplevel::sendClose()
 *
 * Sends a close event to the client. The client may choose to ignore the event.
 */

/*!
 * Sends a close event to the client. The client may choose to ignore the event.
 */
void WaylandXdgToplevel::sendClose()
{
    Q_D(WaylandXdgToplevel);
    d->send_close();
}

/*!
 * \qmlmethod void AuroraCompositor::XdgToplevel::sendMaximized(size size)
 *
 * Convenience for sending a configure event with the maximized state set, and
 * fullscreen and resizing removed. The activated state is left in its current state.
 *
 * \a size is the new size of the window.
 */

/*!
 * Convenience for sending a configure event with the maximized state set, and
 * fullscreen and resizing removed. The activated state is left in its current state.
 *
 * \a size is the new size of the window.
 */
uint WaylandXdgToplevel::sendMaximized(const QSize &size)
{
    Q_D(WaylandXdgToplevel);
    WaylandXdgToplevelPrivate::ConfigureEvent conf = d->lastSentConfigure();

    if (!conf.states.contains(WaylandXdgToplevel::State::MaximizedState))
        conf.states.append(WaylandXdgToplevel::State::MaximizedState);
    conf.states.removeOne(WaylandXdgToplevel::State::FullscreenState);
    conf.states.removeOne(WaylandXdgToplevel::State::ResizingState);

    return sendConfigure(size, conf.states);
}

/*!
 * \qmlmethod void AuroraCompositor::XdgToplevel::sendUnmaximized(size size)
 *
 * Convenience for sending a configure event with the maximized, fullscreen and
 * resizing states removed, and fullscreen and resizing removed. The activated
 * state is left in its current state.
 *
 * \a size is the new size of the window. If \a size is zero, the client decides the size.
 */

/*!
 * Convenience for sending a configure event with the maximized, fullscreen and
 * resizing states removed, and fullscreen and resizing removed. The activated
 * state is left in its current state.
 *
 * \a size is the new size of the window. If \a size is zero, the client decides the size.
 */
uint WaylandXdgToplevel::sendUnmaximized(const QSize &size)
{
    Q_D(WaylandXdgToplevel);
    WaylandXdgToplevelPrivate::ConfigureEvent conf = d->lastSentConfigure();

    conf.states.removeOne(WaylandXdgToplevel::State::MaximizedState);
    conf.states.removeOne(WaylandXdgToplevel::State::FullscreenState);
    conf.states.removeOne(WaylandXdgToplevel::State::ResizingState);

    return sendConfigure(size, conf.states);

}

/*!
 * \qmlmethod void AuroraCompositor::XdgToplevel::sendFullscreen(size size)
 *
 * Convenience for sending a configure event with the fullscreen state set, and
 * maximized and resizing removed. The activated state is left in its current state.
 *
 * \sa sendUnmaximized
 *
 * \a size is the new size of the window.
 */

/*!
 * Convenience for sending a configure event with the fullscreen state set, and
 * maximized and resizing removed. The activated state is left in its current state.
 *
 * \sa sendUnmaximized
 *
 * \a size is the new size of the window.
 */
uint WaylandXdgToplevel::sendFullscreen(const QSize &size)
{
    Q_D(WaylandXdgToplevel);
    WaylandXdgToplevelPrivate::ConfigureEvent conf = d->lastSentConfigure();

    if (!conf.states.contains(WaylandXdgToplevel::State::FullscreenState))
        conf.states.append(WaylandXdgToplevel::State::FullscreenState);
    conf.states.removeOne(WaylandXdgToplevel::State::MaximizedState);
    conf.states.removeOne(WaylandXdgToplevel::State::ResizingState);

    return sendConfigure(size, conf.states);
}

/*!
 * \qmlmethod void AuroraCompositor::XdgToplevel::sendResizing(size maxSize)
 *
 * Convenience for sending a configure event with the resizing state set, and
 * maximized and fullscreen removed. The activated state is left in its current state.
 *
 * \a maxSize is the new size of the window.
 */

/*!
 * Convenience for sending a configure event with the resizing state set, and
 * maximized and fullscreen removed. The activated state is left in its current state.
 *
 * \a maxSize is the new size of the window.
 */
uint WaylandXdgToplevel::sendResizing(const QSize &maxSize)
{
    Q_D(WaylandXdgToplevel);
    WaylandXdgToplevelPrivate::ConfigureEvent conf = d->lastSentConfigure();

    if (!conf.states.contains(WaylandXdgToplevel::State::ResizingState))
        conf.states.append(WaylandXdgToplevel::State::ResizingState);
    conf.states.removeOne(WaylandXdgToplevel::State::MaximizedState);
    conf.states.removeOne(WaylandXdgToplevel::State::FullscreenState);

    return sendConfigure(maxSize, conf.states);
}

/*!
 * Returns the surface role for the WaylandToplevel.
 */
WaylandSurfaceRole *WaylandXdgToplevel::role()
{
    return &WaylandXdgToplevelPrivate::s_role;
}

/*!
 * Returns the WaylandXdgToplevel corresponding to the \a resource.
 */
WaylandXdgToplevel *WaylandXdgToplevel::fromResource(wl_resource *resource)
{
    if (auto p = Internal::fromResource<WaylandXdgToplevelPrivate *>(resource))
        return p->q_func();
    return nullptr;
}

/*!
 * \qmlsignal AuroraCompositor::XdgShell::xdgSurfaceCreated(XdgSurface xdgSurface)
 *
 * This signal is emitted when the client has created a \c xdg_surface.
 * Note that \a xdgSurface is not mapped, i.e. according to the \c xdg-shell
 * protocol it should not be displayed, until it has received a role object.
 *
 * \sa toplevelCreated(), popupCreated()
 */

/*!
 * \fn void WaylandXdgShell::xdgSurfaceCreated(WaylandXdgSurface *xdgSurface)
 *
 * This signal is emitted when the client has created a \c xdg_surface.
 * Note that \a xdgSurface is not mapped, i.e. according to the \c xdg-shell
 * protocol it should not be displayed, until it has received a role object.
 *
 * \sa toplevelCreated(), popupCreated()
 */

/*!
 * \qmlsignal AuroraCompositor::XdgShell::toplevelCreated(XdgToplevel toplevel, XdgSurface xdgSurface)
 *
 * This signal is emitted when the client has created a \c xdg_toplevel.
 * A common use case is to let the handler of this signal instantiate a ShellSurfaceItem or
 * WaylandQuickItem for displaying \a toplevel in a QtQuick scene.
 *
 * \a xdgSurface is the XdgSurface \a toplevel is the role object for.
 */

/*!
 * \fn void WaylandXdgShell::toplevelCreated(WaylandXdgToplevel *toplevel, WaylandXdgSurface *xdgSurface)
 *
 * This signal is emitted when the client has created a \c xdg_toplevel.
 * A common use case is to let the handler of this signal instantiate a WaylandShellSurfaceItem or
 * WaylandQuickItem for displaying \a toplevel in a QtQuick scene.
 *
 * \a xdgSurface is the XdgSurface \a toplevel is the role object for.
 */

/*!
 * \qmlsignal AuroraCompositor::XdgShell::popupCreated(XdgPopup popup, XdgSurface xdgSurface)
 *
 * This signal is emitted when the client has created a \c xdg_popup.
 * A common use case is to let the handler of this signal instantiate a ShellSurfaceItem or
 * WaylandQuickItem for displaying \a popup in a QtQuick scene.
 *
 * \a xdgSurface is the XdgSurface \a popup is the role object for.
 */

/*!
 * \fn void WaylandXdgShell::popupCreated(WaylandXdgPopup *popup, WaylandXdgSurface *xdgSurface)
 *
 * This signal is emitted when the client has created a \c xdg_popup.
 * A common use case is to let the handler of this signal instantiate a WaylandShellSurfaceItem or
 * WaylandQuickItem for displaying \a popup in a QtQuick scene.
 *
 * \a xdgSurface is the XdgSurface \a popup is the role object for.
 */

/*!
 * \qmlsignal AuroraCompositor::XdgShell::pong(int serial)
 *
 * This signal is emitted when the client has responded to a ping event with serial, \a serial.
 *
 * \sa ping()
 */

/*!
 * \fn void WaylandXdgShell::pong(uint serial)
 *
 * This signal is emitted when the client has responded to a ping event with serial, \a serial.
 *
 * \sa WaylandXdgShell::ping()
 */

QList<int> WaylandXdgToplevel::statesAsInts() const
{
   QList<int> list;
   const auto s = states();
   list.reserve(s.size());
   for (auto state : s) {
       list << static_cast<int>(state);
   }
   return list;
}

WaylandSurfaceRole WaylandXdgToplevelPrivate::s_role("xdg_toplevel");

WaylandXdgToplevelPrivate::WaylandXdgToplevelPrivate(WaylandXdgToplevel *self, WaylandXdgSurface *xdgSurface, const WaylandResource &resource)
    : m_xdgSurface(xdgSurface)
    , q_ptr(self)
{
    init(resource.resource());
}

void WaylandXdgToplevelPrivate::handleAckConfigure(uint serial)
{
    Q_Q(WaylandXdgToplevel);
    ConfigureEvent config;
    Q_FOREVER {
        if (m_pendingConfigures.empty()) {
            qWarning("Toplevel received an unexpected ack_configure!");
            return;
        }

        // This won't work unless there always is a toplevel.configure for each xdgsurface.configure
        config = m_pendingConfigures.takeFirst();

        if (config.serial == serial)
            break;
    }

    QList<uint> changedStates;
    std::set_symmetric_difference(
                m_lastAckedConfigure.states.begin(), m_lastAckedConfigure.states.end(),
                config.states.begin(), config.states.end(),
                std::back_inserter(changedStates));

    m_lastAckedConfigure = config;

    for (const uint state : qAsConst(changedStates)) {
        switch (state) {
        case state_maximized:
            emit q->maximizedChanged();
            break;
        case state_fullscreen:
            emit q->fullscreenChanged();
            break;
        case state_resizing:
            emit q->resizingChanged();
            break;
        case state_activated:
            emit q->activatedChanged();
            break;
        }
    }

    if (!changedStates.empty())
        emit q->statesChanged();
}

void WaylandXdgToplevelPrivate::handleFocusLost()
{
    Q_Q(WaylandXdgToplevel);
    WaylandXdgToplevelPrivate::ConfigureEvent current = lastSentConfigure();
    current.states.removeOne(WaylandXdgToplevel::State::ActivatedState);
    q->sendConfigure(current.size, current.states);
}

void WaylandXdgToplevelPrivate::handleFocusReceived()
{
    Q_Q(WaylandXdgToplevel);
    WaylandXdgToplevelPrivate::ConfigureEvent current = lastSentConfigure();
    if (!current.states.contains(WaylandXdgToplevel::State::ActivatedState)) {
        current.states.push_back(WaylandXdgToplevel::State::ActivatedState);
        q->sendConfigure(current.size, current.states);
    }
}

Qt::Edges WaylandXdgToplevelPrivate::convertToEdges(resize_edge edge)
{
    return Qt::Edges(((edge & 0b1100) >> 1) | ((edge & 0b0010) << 2) | (edge & 0b0001));
}

void WaylandXdgToplevelPrivate::xdg_toplevel_destroy_resource(PrivateServer::xdg_toplevel::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(WaylandXdgToplevel);
    delete q;
}

void WaylandXdgToplevelPrivate::xdg_toplevel_destroy(PrivateServer::xdg_toplevel::Resource *resource)
{
    if (Q_UNLIKELY(m_decoration))
        qWarning() << "Client error: xdg_toplevel destroyed before its decoration object";

    wl_resource_destroy(resource->handle);
    //TODO: Should the xdg surface be desroyed as well? Or is it allowed to recreate a new toplevel for it?
}

void WaylandXdgToplevelPrivate::xdg_toplevel_set_parent(PrivateServer::xdg_toplevel::Resource *resource, wl_resource *parent)
{
    Q_UNUSED(resource);
    WaylandXdgToplevel *parentToplevel = WaylandXdgToplevel::fromResource(parent);

    Q_Q(WaylandXdgToplevel);

    if (m_parentToplevel != parentToplevel) {
        m_parentToplevel = parentToplevel;
        emit q->parentToplevelChanged();
    }

    if (m_parentToplevel && m_xdgSurface->windowType() != Qt::WindowType::SubWindow) {
        // There's a parent now, which means the surface is transient
        WaylandXdgSurfacePrivate::get(m_xdgSurface)->setWindowType(Qt::WindowType::SubWindow);
    } else if (!m_parentToplevel && m_xdgSurface->windowType() != Qt::WindowType::Window) {
        // When the surface has no parent it is toplevel
        WaylandXdgSurfacePrivate::get(m_xdgSurface)->setWindowType(Qt::WindowType::Window);
    }
}

void WaylandXdgToplevelPrivate::xdg_toplevel_set_title(PrivateServer::xdg_toplevel::Resource *resource, const QString &title)
{
    Q_UNUSED(resource);
    if (title == m_title)
        return;
    Q_Q(WaylandXdgToplevel);
    m_title = title;
    emit q->titleChanged();
}

void WaylandXdgToplevelPrivate::xdg_toplevel_set_app_id(PrivateServer::xdg_toplevel::Resource *resource, const QString &app_id)
{
    Q_UNUSED(resource);
    if (app_id == m_appId)
        return;
    Q_Q(WaylandXdgToplevel);
    m_appId = app_id;
    emit q->appIdChanged();
}

void WaylandXdgToplevelPrivate::xdg_toplevel_show_window_menu(PrivateServer::xdg_toplevel::Resource *resource, wl_resource *seatResource, uint32_t serial, int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    QPoint position(x, y);
    auto seat = WaylandSeat::fromSeatResource(seatResource);
    Q_Q(WaylandXdgToplevel);
    emit q->showWindowMenu(seat, position);
}

void WaylandXdgToplevelPrivate::xdg_toplevel_move(Resource *resource, wl_resource *seatResource, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_Q(WaylandXdgToplevel);
    WaylandSeat *seat = WaylandSeat::fromSeatResource(seatResource);
    emit q->startMove(seat);
}

void WaylandXdgToplevelPrivate::xdg_toplevel_resize(PrivateServer::xdg_toplevel::Resource *resource, wl_resource *seatResource, uint32_t serial, uint32_t edges)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_Q(WaylandXdgToplevel);
    WaylandSeat *seat = WaylandSeat::fromSeatResource(seatResource);
    emit q->startResize(seat, convertToEdges(resize_edge(edges)));
}

void WaylandXdgToplevelPrivate::xdg_toplevel_set_max_size(PrivateServer::xdg_toplevel::Resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(resource);

    QSize maxSize(width, height);
    if (width == 0 && height == 0)
        maxSize = QSize(); // Wayland size of zero means unspecified which best translates to invalid

    if (m_maxSize == maxSize)
        return;

    if (width < 0 || height < 0) {
        // The spec says raise a protocol error, but there's no matching error defined
        qWarning() << "Received a xdg_toplevel.set_max_size request with a negative size";
        return;
    }

    if (m_minSize.isValid() && maxSize.isValid() &&
            (maxSize.width() < m_minSize.width() || maxSize.height() < m_minSize.height())) {
        // The spec says raise a protocol error, but there's no matching error defined
        qWarning() << "Received a xdg_toplevel.set_max_size request with a size smaller than the minimium size";
        return;
    }

    m_maxSize = maxSize;

    Q_Q(WaylandXdgToplevel);
    emit q->maxSizeChanged();
}

void WaylandXdgToplevelPrivate::xdg_toplevel_set_min_size(PrivateServer::xdg_toplevel::Resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(resource);

    QSize minSize(width, height);
    if (width == 0 && height == 0)
        minSize = QSize(); // Wayland size of zero means unspecified

    if (m_minSize == minSize)
        return;

    if (width < 0 || height < 0) {
        // The spec says raise a protocol error, but there's no matching error defined
        qWarning() << "Received a xdg_toplevel.set_min_size request with a negative size";
        return;
    }

    if (m_maxSize.isValid() && minSize.isValid() &&
            (minSize.width() > m_maxSize.width() || minSize.height() > m_maxSize.height())) {
        // The spec says raise a protocol error, but there's no matching error defined
        qWarning() << "Received a xdg_toplevel.set_min_size request with a size larger than the maximum size";
        return;
    }

    m_minSize = minSize;

    Q_Q(WaylandXdgToplevel);
    emit q->minSizeChanged();
}

void WaylandXdgToplevelPrivate::xdg_toplevel_set_maximized(PrivateServer::xdg_toplevel::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(WaylandXdgToplevel);
    emit q->setMaximized();
}

void WaylandXdgToplevelPrivate::xdg_toplevel_unset_maximized(PrivateServer::xdg_toplevel::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(WaylandXdgToplevel);
    emit q->unsetMaximized();
}

void WaylandXdgToplevelPrivate::xdg_toplevel_set_fullscreen(PrivateServer::xdg_toplevel::Resource *resource, wl_resource *output_res)
{
    Q_UNUSED(resource);
    Q_Q(WaylandXdgToplevel);
    WaylandOutput *output = output_res ? WaylandOutput::fromResource(output_res) : nullptr;
    emit q->setFullscreen(output);
}

void WaylandXdgToplevelPrivate::xdg_toplevel_unset_fullscreen(PrivateServer::xdg_toplevel::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(WaylandXdgToplevel);
    emit q->unsetFullscreen();
}

void WaylandXdgToplevelPrivate::xdg_toplevel_set_minimized(PrivateServer::xdg_toplevel::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(WaylandXdgToplevel);
    emit q->setMinimized();
}

/*!
 * \qmltype XdgPopup
 * \instantiates WaylandXdgPopup
 * \inqmlmodule Aurora.Compositor.XdgShell
 * \since 5.12
 * \brief XdgPopup represents the popup specific parts of and xdg surface.
 *
 * This type is part of the \l{XdgShell} extension and provides a way to extend
 * extend the functionality of an \l{XdgSurface} with features
 * specific to desktop-style menus for an xdg surface.
 *
 * It corresponds to the Wayland interface \c xdg_popup.
 */

/*!
 * \class WaylandXdgPopup
 * \inmodule AuroraCompositor
 * \since 5.12
 * \brief The WaylandXdgPopup class represents the popup specific parts of an xdg surface.
 *
 * This class is part of the WaylandXdgShell extension and provides a way to
 * extend the functionality of a WaylandXdgSurface with features
 * specific to desktop-style menus for an xdg surface.
 *
 * It corresponds to the Wayland interface \c xdg_popup.
 */

/*!
 * Constructs a WaylandXdgPopup.
 */
WaylandXdgPopup::WaylandXdgPopup(WaylandXdgSurface *xdgSurface, WaylandXdgSurface *parentXdgSurface,
                                   WaylandXdgPositioner *positioner, WaylandResource &resource)
    : QObject()
    , d_ptr(new WaylandXdgPopupPrivate(this, xdgSurface, parentXdgSurface, positioner, resource))
{
}

WaylandXdgPopup::~WaylandXdgPopup()
{
}

/*!
 * \qmlproperty XdgSurface AuroraCompositor::XdgPopup::xdgSurface
 *
 * This property holds the XdgSurface associated with this XdgPopup.
 */

/*!
 * \property WaylandXdgPopup::xdgSurface
 *
 * This property holds the WaylandXdgSurface associated with this WaylandXdgPopup.
 */
WaylandXdgSurface *WaylandXdgPopup::xdgSurface() const
{
    Q_D(const WaylandXdgPopup);
    return d->m_xdgSurface;
}

/*!
 * \qmlproperty XdgSurface AuroraCompositor::XdgPopup::parentXdgSurface
 *
 * This property holds the XdgSurface associated with the parent of this XdgPopup.
 */

/*!
 * \property WaylandXdgPopup::parentXdgSurface
 *
 * This property holds the WaylandXdgSurface associated with the parent of this
 * WaylandXdgPopup.
 */
WaylandXdgSurface *WaylandXdgPopup::parentXdgSurface() const
{
    Q_D(const WaylandXdgPopup);
    return d->m_parentXdgSurface;
}

/*!
 * \qmlproperty rect AuroraCompositor::XdgPopup::configuredGeometry
 *
 * The window geometry the popup received in the configure event. Relative to the
 * upper left corner of the parent surface.
 */

/*!
 * \property WaylandXdgPopup::configuredGeometry
 *
 * The window geometry the popup received in the configure event. Relative to the
 * upper left corner of the parent surface.
 */
QRect WaylandXdgPopup::configuredGeometry() const
{
    Q_D(const WaylandXdgPopup);
    return d->m_geometry;
}

/*!
 * \qmlproperty rect AuroraCompositor::XdgPopup::anchorRect
 *
 * The anchor rectangle relative to the parent window geometry that the child
 * surface should be placed relative to.
 */

/*!
 * \property WaylandXdgPopup::anchorRect
 *
 * Returns the anchor rectangle relative to the parent window geometry that the child
 * surface should be placed relative to.
 */
QRect WaylandXdgPopup::anchorRect() const
{
    Q_D(const WaylandXdgPopup);
    return d->m_positionerData.anchorRect;
}

/*!
 * \qmlproperty enumeration AuroraCompositor::XdgPopup::anchorEdges
 *
 * This property holds the set of edges on the anchor rect that the child surface should be placed
 * relative to. If no edges are specified in a direction, the anchor point should be
 * centered between the edges.
 *
 * The possible values are:
 * \value Qt.TopEdge The top edge of the rectangle.
 * \value Qt.LeftEdge The left edge of the rectangle.
 * \value Qt.RightEdge The right edge of the rectangle.
 * \value Qt.BottomEdge The bottom edge of the rectangle.
 */

/*!
 * \property WaylandXdgPopup::anchorEdges
 *
 * Returns the set of edges on the anchor rect that the child surface should be placed
 * relative to. If no edges are specified in a direction, the anchor point should be
 * centered between the edges.
 */
Qt::Edges WaylandXdgPopup::anchorEdges() const
{
    Q_D(const WaylandXdgPopup);
    return d->m_positionerData.anchorEdges;
}

/*!
 * \qmlproperty rect AuroraCompositor::XdgPopup::gravityEdges
 *
 * Specifies in what direction the surface should be positioned, relative to the anchor
 * point.
 *
 * The possible values are:
 * \value Qt.TopEdge The surface should slide towards the top of the screen.
 * \value Qt.LeftEdge The surface should slide towards the left of the screen.
 * \value Qt.RightEdge The surface should slide towards the right of the screen.
 * \value Qt.BottomEdge The surface should slide towards the bottom of the screen.
 */

/*!
 * \property WaylandXdgPopup::gravityEdges
 *
 * Specifies in what direction the surface should be positioned, relative to the anchor
 * point.
 */
Qt::Edges WaylandXdgPopup::gravityEdges() const
{
    Q_D(const WaylandXdgPopup);
    return d->m_positionerData.gravityEdges;
}

/*!
 * \qmlproperty enumeration AuroraCompositor::XdgPopup::slideConstraints
 *
 * This property holds the orientations in which the child should slide to fit within the screen.
 *
 * Possible values:
 * \value Qt.Horizontal Horizontal
 * \value Qt.Vertical Vertical
 */

/*!
 * \property WaylandXdgPopup::slideConstraints
 *
 * This property holds the orientations in which the child should slide to fit within the screen.
 */
Qt::Orientations WaylandXdgPopup::slideConstraints() const
{
    Q_D(const WaylandXdgPopup);
    const uint flags = d->m_positionerData.constraintAdjustments;

    Qt::Orientations constraints = {};

    if (flags & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X)
        constraints |= Qt::Horizontal;
    if (flags & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y)
        constraints |= Qt::Vertical;

    return constraints;
}

/*!
 * \qmlproperty enumeration AuroraCompositor::XdgPopup::flipConstraints
 *
 * This property holds the orientations in which the child should flip to fit within the screen.
 *
 * Possible values:
 * \value Qt.Horizontal Horizontal
 * \value Qt.Vertical Vertical
 */

/*!
 * \property WaylandXdgPopup::flipConstraints
 *
 * This property holds the orientations in which the child should flip to fit within the screen.
 */
Qt::Orientations WaylandXdgPopup::flipConstraints() const
{
    Q_D(const WaylandXdgPopup);
    const uint flags = d->m_positionerData.constraintAdjustments;

    Qt::Orientations constraints = {};

    if (flags & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X)
        constraints |= Qt::Horizontal;
    if (flags & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y)
        constraints |= Qt::Vertical;

    return constraints;
}

/*!
 * \qmlproperty enumeration AuroraCompositor::XdgPopup::resizeConstraints
 *
 * This property holds the orientations in which the child should resize to fit within the screen.
 *
 * Possible values:
 * \value Qt.Horizontal Horizontal
 * \value Qt.Vertical Vertical
 */

/*!
 * \property WaylandXdgPopup::resizeConstraints
 *
 * This property holds the orientations in which the child should resize to fit within the screen.
 */
Qt::Orientations WaylandXdgPopup::resizeConstraints() const
{
    Q_D(const WaylandXdgPopup);
    const uint flags = d->m_positionerData.constraintAdjustments;

    Qt::Orientations constraints = {};

    if (flags & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_X)
        constraints |= Qt::Horizontal;
    if (flags & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_Y)
        constraints |= Qt::Vertical;

    return constraints;
}

/*!
 * \qmlproperty point AuroraCompositor::XdgPopup::offset
 *
 * The position relative to the position of the anchor on the anchor rectangle and
 * the anchor on the surface.
 */

/*!
 * \property WaylandXdgPopup::offset
 *
 * Returns the surface position relative to the position of the anchor on the anchor
 * rectangle and the anchor on the surface.
 */
QPoint WaylandXdgPopup::offset() const
{
    Q_D(const WaylandXdgPopup);
    return d->m_positionerData.offset;
}

/*!
 * \qmlproperty size AuroraCompositor::XdgPopup::positionerSize
 *
 * The size requested for the window geometry by the positioner object.
 */

/*!
 * \property WaylandXdgPopup::positionerSize
 *
 * Returns the size requested for the window geometry by the positioner object.
 */
QSize WaylandXdgPopup::positionerSize() const
{
    Q_D(const WaylandXdgPopup);
    return d->m_positionerData.size;
}

/*!
 * \qmlproperty point AuroraCompositor::XdgPopup::unconstrainedPosition
 *
 * The position of the surface relative to the parent window geometry if the surface
 * is not constrained. I.e. when not moved to fit inside the screen or similar.
 */

/*!
 * \property WaylandXdgPopup::unconstrainedPosition
 *
 * The position of the surface relative to the parent window geometry if the surface
 * is not constrained. I.e. when not moved to fit inside the screen or similar.
 */
QPoint WaylandXdgPopup::unconstrainedPosition() const
{
    Q_D(const WaylandXdgPopup);
    return d->m_positionerData.unconstrainedPosition();
}

/*!
 * \qmlmethod int AuroraCompositor::XdgPopup::sendConfigure(rect geometry)
 *
 * Sends a configure event to the client. \a geometry contains the window geometry
 * relative to the upper left corner of the window geometry of the parent surface.
 *
 * This implicitly sends a configure event to the corresponding XdgSurface as well.
 */

/*!
 * Sends a configure event to the client. \a geometry contains the window geometry
 * relative to the upper left corner of the window geometry of the parent surface.
 *
 * This implicitly sends a configure event to the corresponding WaylandXdgSurface
 * as well.
 */
uint WaylandXdgPopup::sendConfigure(const QRect &geometry)
{
    Q_D(WaylandXdgPopup);
    return d->sendConfigure(geometry);
}

/*!
 * \qmlmethod void AuroraCompositor::XdgPopup::sendPopupDone()
 * \since 5.14
 *
 * Dismiss the popup. According to the \c xdg-shell protocol this should make the
 * client destroy the popup.
 */

/*!
 * \since 5.14
 *
 * Dismiss the popup. According to the \c xdg-shell protocol this should make the
 * client destroy the popup.
 */
void WaylandXdgPopup::sendPopupDone()
{
    Q_D(WaylandXdgPopup);
    d->send_popup_done();
}

/*!
 * Returns the surface role for the WaylandPopup.
 */
WaylandSurfaceRole *WaylandXdgPopup::role()
{
    return &WaylandXdgPopupPrivate::s_role;
}

WaylandXdgPopupPrivate::WaylandXdgPopupPrivate(WaylandXdgPopup *self,
                                               WaylandXdgSurface *xdgSurface, WaylandXdgSurface *parentXdgSurface,
                                               WaylandXdgPositioner *positioner, const WaylandResource &resource)
    : q_ptr(self)
    , m_xdgSurface(xdgSurface)
    , m_parentXdgSurface(parentXdgSurface)
    , m_positionerData(positioner->m_data)
{
    Q_ASSERT(m_positionerData.isComplete());
    init(resource.resource());

    WaylandXdgSurfacePrivate::get(m_xdgSurface)->setWindowType(Qt::WindowType::Popup);

    //TODO: Need an API for sending a different initial configure
    sendConfigure(QRect(m_positionerData.unconstrainedPosition(), m_positionerData.size));
}

WaylandXdgPopupPrivate::~WaylandXdgPopupPrivate()
{
}

void WaylandXdgPopupPrivate::handleAckConfigure(uint serial)
{
    Q_Q(WaylandXdgPopup);
    ConfigureEvent config;
    Q_FOREVER {
        if (m_pendingConfigures.empty()) {
            qWarning("Popup received an unexpected ack_configure!");
            return;
        }

        // This won't work unless there always is a popup.configure for each xdgsurface.configure
        config = m_pendingConfigures.takeFirst();

        if (config.serial == serial)
            break;
    }

    if (m_geometry == config.geometry)
        return;

    m_geometry = config.geometry;
    emit q->configuredGeometryChanged();
}

uint WaylandXdgPopupPrivate::sendConfigure(const QRect &geometry)
{
    uint32_t serial = m_xdgSurface->surface()->compositor()->nextSerial();
    m_pendingConfigures.append(WaylandXdgPopupPrivate::ConfigureEvent{geometry, serial});
    send_configure(geometry.x(), geometry.y(), geometry.width(), geometry.height());
    WaylandXdgSurfacePrivate::get(m_xdgSurface)->send_configure(serial);
    return serial;
}

void WaylandXdgPopupPrivate::xdg_popup_destroy(PrivateServer::xdg_popup::Resource *resource)
{
    Q_UNUSED(resource);
    qWarning() << Q_FUNC_INFO << "Not implemented"; //TODO
}

void WaylandXdgPopupPrivate::xdg_popup_grab(PrivateServer::xdg_popup::Resource *resource, wl_resource *seat, uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_UNUSED(seat);
    qWarning() << Q_FUNC_INFO << "Not implemented"; //TODO
    //switch keyboard focus
    //eventually send configure with activated.
}

WaylandSurfaceRole WaylandXdgPopupPrivate::s_role("xdg_popup");

WaylandXdgPositionerData::WaylandXdgPositionerData()
    : offset(0, 0)
{}

bool WaylandXdgPositionerData::isComplete() const
{
    return size.width() > 0 && size.height() > 0 && anchorRect.size().width() > 0 && anchorRect.size().height() > 0;
}

QPoint WaylandXdgPositionerData::anchorPoint() const
{
    int yPosition = 0;
    if (anchorEdges & Qt::TopEdge)
        yPosition = anchorRect.top();
    else if (anchorEdges & Qt::BottomEdge)
        yPosition = anchorRect.bottom() + 1;
    else
        yPosition = anchorRect.top() + anchorRect.height() / 2;

    int xPosition = 0;
    if (anchorEdges & Qt::LeftEdge)
        xPosition = anchorRect.left();
    else if (anchorEdges & Qt::RightEdge)
        xPosition = anchorRect.right() + 1;
    else
        xPosition = anchorRect.left() + anchorRect.width() / 2;

    return QPoint(xPosition, yPosition);
}

QPoint WaylandXdgPositionerData::unconstrainedPosition() const
{
    int gravityOffsetY = 0;
    if (gravityEdges & Qt::TopEdge)
        gravityOffsetY = -size.height();
    else if (!(gravityEdges & Qt::BottomEdge))
        gravityOffsetY = -size.height() / 2;

    int gravityOffsetX = 0;
    if (gravityEdges & Qt::LeftEdge)
        gravityOffsetX = -size.width();
    else if (!(gravityEdges & Qt::RightEdge))
        gravityOffsetX = -size.width() / 2;

    QPoint gravityOffset(gravityOffsetX, gravityOffsetY);
    return anchorPoint() + gravityOffset + offset;
}

WaylandXdgPositioner::WaylandXdgPositioner(const WaylandResource &resource)
{
    init(resource.resource());
}

void WaylandXdgPositioner::xdg_positioner_destroy_resource(PrivateServer::xdg_positioner::Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void WaylandXdgPositioner::xdg_positioner_destroy(PrivateServer::xdg_positioner::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void WaylandXdgPositioner::xdg_positioner_set_size(PrivateServer::xdg_positioner::Resource *resource, int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) {
        wl_resource_post_error(resource->handle, XDG_POSITIONER_ERROR_INVALID_INPUT,
                               "xdg_positioner.set_size requested with non-positive dimensions");
        return;
    }

    QSize size(width, height);
    m_data.size = size;
}

void WaylandXdgPositioner::xdg_positioner_set_anchor_rect(PrivateServer::xdg_positioner::Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) {
        wl_resource_post_error(resource->handle, XDG_POSITIONER_ERROR_INVALID_INPUT,
                               "xdg_positioner.set_anchor_rect requested with non-positive dimensions");
        return;
    }

    QRect anchorRect(x, y, width, height);
    m_data.anchorRect = anchorRect;
}

void WaylandXdgPositioner::xdg_positioner_set_anchor(PrivateServer::xdg_positioner::Resource *resource, uint32_t anchor)
{
    Qt::Edges anchorEdges = convertToEdges(xdg_positioner::anchor(anchor));

    if ((anchorEdges & Qt::BottomEdge && anchorEdges & Qt::TopEdge) ||
            (anchorEdges & Qt::LeftEdge && anchorEdges & Qt::RightEdge)) {
        wl_resource_post_error(resource->handle, XDG_POSITIONER_ERROR_INVALID_INPUT,
                               "xdg_positioner.set_anchor requested with parallel edges");
        return;
    }

    m_data.anchorEdges = anchorEdges;
}

void WaylandXdgPositioner::xdg_positioner_set_gravity(PrivateServer::xdg_positioner::Resource *resource, uint32_t gravity)
{
    Qt::Edges gravityEdges = convertToEdges(xdg_positioner::gravity(gravity));

    if ((gravityEdges & Qt::BottomEdge && gravityEdges & Qt::TopEdge) ||
            (gravityEdges & Qt::LeftEdge && gravityEdges & Qt::RightEdge)) {
        wl_resource_post_error(resource->handle, XDG_POSITIONER_ERROR_INVALID_INPUT,
                               "xdg_positioner.set_gravity requested with parallel edges");
        return;
    }

    m_data.gravityEdges = gravityEdges;
}

void WaylandXdgPositioner::xdg_positioner_set_constraint_adjustment(PrivateServer::xdg_positioner::Resource *resource, uint32_t constraint_adjustment)
{
    Q_UNUSED(resource);
    m_data.constraintAdjustments = constraint_adjustment;
}

void WaylandXdgPositioner::xdg_positioner_set_offset(PrivateServer::xdg_positioner::Resource *resource, int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    m_data.offset = QPoint(x, y);
}

WaylandXdgPositioner *WaylandXdgPositioner::fromResource(wl_resource *resource)
{
    return Internal::fromResource<WaylandXdgPositioner *>(resource);
}

Qt::Edges WaylandXdgPositioner::convertToEdges(anchor anchor)
{
    switch (anchor) {
    case anchor_none:
        return Qt::Edges();
    case anchor_top:
        return Qt::TopEdge;
    case anchor_bottom:
        return Qt::BottomEdge;
    case anchor_left:
        return Qt::LeftEdge;
    case anchor_right:
        return Qt::RightEdge;
    case anchor_top_left:
        return Qt::TopEdge | Qt::LeftEdge;
    case anchor_bottom_left:
        return Qt::BottomEdge | Qt::LeftEdge;
    case anchor_top_right:
        return Qt::TopEdge | Qt::RightEdge;
    case anchor_bottom_right:
        return Qt::BottomEdge | Qt::RightEdge;
    default:
        qWarning() << "Unknown Wayland xdg edge" << anchor;
        return Qt::Edges();
    }
}

Qt::Edges WaylandXdgPositioner::convertToEdges(WaylandXdgPositioner::gravity gravity)
{
    return convertToEdges(anchor(gravity));
}


} // namespace Compositor

} // namespace Aurora
