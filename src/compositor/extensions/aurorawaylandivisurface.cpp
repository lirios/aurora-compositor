// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandivisurface.h"
#include "aurorawaylandivisurface_p.h"
#include "aurorawaylandiviapplication_p.h"
#if LIRI_FEATURE_aurora_compositor_quick
#include "aurorawaylandivisurfaceintegration_p.h"
#endif

#include <LiriAuroraCompositor/WaylandResource>
#include <QDebug>

#include <LiriAuroraCompositor/private/aurorawaylandutils_p.h>

namespace Aurora {

namespace Compositor {

WaylandSurfaceRole WaylandIviSurfacePrivate::s_role("ivi_surface");

/*!
 * \qmltype IviSurface
 * \instantiates WaylandIviSurface
 * \inqmlmodule Aurora.Compositor.IviApplication
 * \since 5.8
 * \brief Provides a simple way to identify and resize a surface.
 *
 * This type is part of the \l{IviApplication} extension and provides a way to extend
 * the functionality of an existing WaylandSurface with a way to resize and identify it.
 *
 * It corresponds to the Wayland \c ivi_surface interface.
 */

/*!
 * \class WaylandIviSurface
 * \inmodule AuroraCompositor
 * \since 5.8
 * \brief The WaylandIviSurface class provides a simple way to identify and resize a surface.
 *
 * This class is part of the WaylandIviApplication extension and provides a way to
 * extend the functionality of an existing WaylandSurface with a way to resize and identify it.
 *
 * It corresponds to the Wayland \c ivi_surface interface.
 */

/*!
 * Constructs a WaylandIviSurface.
 */
WaylandIviSurface::WaylandIviSurface()
    : WaylandShellSurfaceTemplate<WaylandIviSurface>(*new WaylandIviSurfacePrivate())
{
}

/*!
 * Constructs a WaylandIviSurface for \a surface and initializes it with the
 * given \a application, \a surface, \a iviId, and \a resource.
 */
WaylandIviSurface::WaylandIviSurface(WaylandIviApplication *application, WaylandSurface *surface, uint iviId, const WaylandResource &resource)
    : WaylandShellSurfaceTemplate<WaylandIviSurface>(*new WaylandIviSurfacePrivate())
{
    initialize(application, surface, iviId, resource);
}

/*!
 * \qmlmethod void IviSurface::initialize(IviApplication iviApplication, WaylandSurface surface, int iviId, WaylandResource resource)
 *
 * Initializes the IviSurface, associating it with the given \a iviApplication, \a surface,
 * \a iviId, and \a resource.
 */

/*!
 * Initializes the WaylandIviSurface, associating it with the given \a iviApplication, \a surface,
 * \a iviId, and \a resource.
 */
void WaylandIviSurface::initialize(WaylandIviApplication *iviApplication, WaylandSurface *surface, uint iviId, const WaylandResource &resource)
{
    Q_D(WaylandIviSurface);

    d->m_iviApplication = iviApplication;
    d->m_surface = surface;
    d->m_iviId = iviId;

    d->init(resource.resource());
    setExtensionContainer(surface);

    emit surfaceChanged();
    emit iviIdChanged();

    WaylandCompositorExtension::initialize();
}

/*!
 * \qmlproperty WaylandSurface IviSurface::surface
 *
 * This property holds the surface associated with this IviSurface.
 */

/*!
 * \property WaylandIviSurface::surface
 *
 * This property holds the surface associated with this WaylandIviSurface.
 */
WaylandSurface *WaylandIviSurface::surface() const
{
    Q_D(const WaylandIviSurface);
    return d->m_surface;
}

/*!
 * \qmlproperty int IviSurface::iviId
 * \readonly
 *
 * This property holds the ivi id id of this IviSurface.
 */

/*!
 * \property WaylandIviSurface::iviId
 *
 * This property holds the ivi id of this WaylandIviSurface.
 */
uint WaylandIviSurface::iviId() const
{
    Q_D(const WaylandIviSurface);
    return d->m_iviId;
}

/*!
 * Returns the Wayland interface for the WaylandIviSurface.
 */
const struct wl_interface *WaylandIviSurface::interface()
{
    return WaylandIviSurfacePrivate::interface();
}

QByteArray WaylandIviSurface::interfaceName()
{
    return WaylandIviSurfacePrivate::interfaceName();
}

/*!
 * Returns the surface role for the WaylandIviSurface.
 */
WaylandSurfaceRole *WaylandIviSurface::role()
{
    return &WaylandIviSurfacePrivate::s_role;
}

/*!
 * Returns the WaylandIviSurface corresponding to the \a resource.
 */
WaylandIviSurface *WaylandIviSurface::fromResource(wl_resource *resource)
{
    if (auto p = Internal::fromResource<WaylandIviSurfacePrivate *>(resource))
        return p->q_func();
    return nullptr;
}

/*!
 * \qmlmethod int IviSurface::sendConfigure(size size)
 *
 * Sends a configure event to the client, telling it to resize the surface to the given \a size.
 */

/*!
 * Sends a configure event to the client, telling it to resize the surface to the given \a size.
 */
void WaylandIviSurface::sendConfigure(const QSize &size)
{
    if (!size.isValid()) {
        qWarning() << "Can't configure ivi_surface with an invalid size" << size;
        return;
    }
    Q_D(WaylandIviSurface);
    d->send_configure(size.width(), size.height());
}

#if LIRI_FEATURE_aurora_compositor_quick
WaylandQuickShellIntegration *WaylandIviSurface::createIntegration(WaylandQuickShellSurfaceItem *item)
{
    return new Internal::IviSurfaceIntegration(item);
}
#endif

/*!
 * \internal
 */
void WaylandIviSurface::initialize()
{
    WaylandShellSurfaceTemplate::initialize();
}

WaylandIviSurfacePrivate::WaylandIviSurfacePrivate()
{
}

void WaylandIviSurfacePrivate::ivi_surface_destroy_resource(PrivateServer::ivi_surface::Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(WaylandIviSurface);
    WaylandIviApplicationPrivate::get(m_iviApplication)->unregisterIviSurface(q);
    delete q;
}

void WaylandIviSurfacePrivate::ivi_surface_destroy(PrivateServer::ivi_surface::Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandivisurface.cpp"
