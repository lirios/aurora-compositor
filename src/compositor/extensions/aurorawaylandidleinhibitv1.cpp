// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/private/aurorawaylandsurface_p.h>

#include "aurorawaylandidleinhibitv1_p.h"

namespace Aurora {

namespace Compositor {

/*!
    \class WaylandIdleInhibitManagerV1
    \inmodule AuroraCompositor
    \since 5.14
    \brief Provides an extension that allows to inhibit the idle behavior of the compositor.
    \sa WaylandSurface::inhibitsIdle

    The WaylandIdleInhibitV1 extension provides a way for a client to inhibit the idle behavior of
    the compositor when a specific surface is visually relevant to the user.

    WaylandIdleInhibitManagerV1 corresponds to the Wayland interface, \c zwp_idle_inhibit_manager_v1.

    Inhibited surfaces have the WaylandSurface::inhibitsIdle property set to \c true.
*/

/*!
    \qmltype IdleInhibitManagerV1
    \instantiates WaylandIdleInhibitManagerV1
    \inqmlmodule Aurora.Compositor
    \since 5.14
    \brief Provides an extension that allows to inhibit the idle behavior of the compositor.
    \sa WaylandSurface::inhibitsIdle

    The IdleInhibitManagerV1 extension provides a way for a client to inhibit the idle behavior of
    the compositor when a specific surface is visually relevant to the user.

    IdleInhibitManagerV1 corresponds to the Wayland interface, \c zwp_idle_inhibit_manager_v1.

    To provide the functionality of the extension in a compositor, create an instance of the
    IdleInhibitManagerV1 component and add it to the list of extensions supported by the compositor:

    \qml
    import Aurora.Compositor

    WaylandCompositor {
        IdleInhibitManagerV1 {
            // ...
        }
    }
    \endqml

    Inhibited surfaces have the WaylandSurface::inhibitsIdle property set to \c true.
*/

/*!
    Constructs a WaylandIdleInhibitManagerV1 object.
*/
WaylandIdleInhibitManagerV1::WaylandIdleInhibitManagerV1()
    : WaylandCompositorExtensionTemplate<WaylandIdleInhibitManagerV1>(*new WaylandIdleInhibitManagerV1Private())
{
}

/*!
    Constructs a WaylandIdleInhibitManagerV1 object for the provided \a compositor.
*/
WaylandIdleInhibitManagerV1::WaylandIdleInhibitManagerV1(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate<WaylandIdleInhibitManagerV1>(compositor, *new WaylandIdleInhibitManagerV1Private())
{
}

/*!
    Destructs a WaylandIdleInhibitManagerV1 object.
*/
WaylandIdleInhibitManagerV1::~WaylandIdleInhibitManagerV1() = default;

/*!
    Initializes the extension.
*/
void WaylandIdleInhibitManagerV1::initialize()
{
    Q_D(WaylandIdleInhibitManagerV1);

    WaylandCompositorExtensionTemplate::initialize();
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qCWarning(gLcAuroraCompositor) << "Failed to find WaylandCompositor when initializing WaylandIdleInhibitManagerV1";
        return;
    }
    d->init(compositor->display(), d->interfaceVersion());
}

/*!
    Returns the Wayland interface for the WaylandIdleInhibitManagerV1.
*/
const wl_interface *WaylandIdleInhibitManagerV1::interface()
{
    return WaylandIdleInhibitManagerV1Private::interface();
}


void WaylandIdleInhibitManagerV1Private::zwp_idle_inhibit_manager_v1_create_inhibitor(Resource *resource, uint id, wl_resource *surfaceResource)
{
    auto *surface = WaylandSurface::fromResource(surfaceResource);
    if (!surface) {
        qCWarning(gLcAuroraCompositor) << "Couldn't find surface requested for creating an inhibitor";
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "invalid wl_surface@%d", wl_resource_get_id(surfaceResource));
        return;
    }

    auto *surfacePrivate = WaylandSurfacePrivate::get(surface);
    if (!surfacePrivate) {
        wl_resource_post_no_memory(resource->handle);
        return;
    }

    auto *inhibitor = new Inhibitor(surface, resource->client(), id, resource->version());
    if (!inhibitor) {
        wl_resource_post_no_memory(resource->handle);
        return;
    }
    surfacePrivate->idleInhibitors.append(inhibitor);

    if (surfacePrivate->idleInhibitors.size() == 1)
        Q_EMIT surface->inhibitsIdleChanged();
}


WaylandIdleInhibitManagerV1Private::Inhibitor::Inhibitor(WaylandSurface *surface,
                                                          wl_client *client,
                                                          quint32 id, quint32 version)
    : PrivateServer::zwp_idle_inhibitor_v1(client, id, qMin<quint32>(version, interfaceVersion()))
    , m_surface(surface)
{
    Q_ASSERT(surface);
}

void WaylandIdleInhibitManagerV1Private::Inhibitor::zwp_idle_inhibitor_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void WaylandIdleInhibitManagerV1Private::Inhibitor::zwp_idle_inhibitor_v1_destroy(Resource *resource)
{
    if (m_surface) {
        auto *surfacePrivate = WaylandSurfacePrivate::get(m_surface.data());
        Q_ASSERT(surfacePrivate->idleInhibitors.contains(this));
        surfacePrivate->idleInhibitors.removeOne(this);

        if (surfacePrivate->idleInhibitors.isEmpty())
            Q_EMIT m_surface.data()->inhibitsIdleChanged();
    }

    wl_resource_destroy(resource->handle);
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandidleinhibitv1.cpp"
