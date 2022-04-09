/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "aurorawaylandiviapplication.h"
#include "aurorawaylandiviapplication_p.h"

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandSurface>
#include <LiriAuroraCompositor/WaylandIviSurface>
#include <LiriAuroraCompositor/WaylandResource>

namespace Aurora {

namespace Compositor {

/*!
 * \qmltype IviApplication
 * \instantiates WaylandIviApplication
 * \inqmlmodule Aurora.Compositor.IviApplication
 * \since 5.8
 * \brief Provides a shell extension for embedded-style user interfaces.
 *
 * The IviApplication extension provides a way to associate an IviSurface
 * with a regular Wayland surface. Using the IviSurface interface, the client can identify
 * itself by giving an ivi id, and the compositor can ask the client to resize.
 *
 * IviApplication corresponds to the Wayland \c ivi_application interface.
 *
 * To provide the functionality of the shell extension in a compositor, create
 * an instance of the IviApplication component and add it to the list of extensions
 * supported by the compositor:
 *
 * \qml
 * import Aurora.Compositor.IviApplication
 *
 * WaylandCompositor {
 *     IviApplication {
 *         onIviSurfaceCreated: {
 *             if (iviSurface.iviId === navigationIviId) {
 *                 // ...
 *             }
 *         }
 *     }
 * }
 * \endqml
 */

/*!
 * \class WaylandIviApplication
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The WaylandIviApplication class is an extension for embedded-style user interfaces.
 *
 * The WaylandIviApplication extension provides a way to associate an WaylandIviSurface
 * with a regular Wayland surface. Using the WaylandIviSurface interface, the client can identify
 * itself by giving an ivi id, and the compositor can ask the client to resize.
 *
 * WaylandIviApplication corresponds to the Wayland \c ivi_application interface.
 */

/*!
 * Constructs a WaylandIviApplication object.
 */
WaylandIviApplication::WaylandIviApplication()
    : WaylandCompositorExtensionTemplate<WaylandIviApplication>(*new WaylandIviApplicationPrivate())
{
}

/*!
 * Constructs a WaylandIviApplication object for the provided \a compositor.
 */
WaylandIviApplication::WaylandIviApplication(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate<WaylandIviApplication>(compositor, *new WaylandIviApplicationPrivate())
{
}

/*!
 * Initializes the shell extension.
 */
void WaylandIviApplication::initialize()
{
    Q_D(WaylandIviApplication);
    WaylandCompositorExtensionTemplate::initialize();

    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find WaylandCompositor when initializing WaylandIviApplication";
        return;
    }

    d->init(compositor->display(), 1);
}

/*!
 * Returns the Wayland interface for the WaylandIviApplication.
 */
const struct wl_interface *WaylandIviApplication::interface()
{
    return WaylandIviApplicationPrivate::interface();
}

/*!
 * \internal
 */
QByteArray WaylandIviApplication::interfaceName()
{
    return WaylandIviApplicationPrivate::interfaceName();
}

/*!
 * \qmlsignal void QtWaylandCompositor::IviApplication::iviSurfaceRequested(WaylandSurface surface, int iviId, WaylandResource resource)
 *
 * This signal is emitted when the client has requested an \c ivi_surface to be associated
 * with \a surface, which is identified by \a iviId. The handler for this signal is
 * expected to create the ivi surface for \a resource and initialize it within the scope of the
 * signal emission. If no ivi surface is created, a default one will be created instead.
 *
 */

/*!
 * \fn void WaylandIviApplication::iviSurfaceRequested(WaylandSurface *surface, uint iviId, const WaylandResource &resource)
 *
 * This signal is emitted when the client has requested an \c ivi_surface to be associated
 * with \a surface, which is identified by \a iviId. The handler for this signal is
 * expected to create the ivi surface for \a resource and initialize it within the scope of the
 * signal emission. If no ivi surface is created, a default one will be created instead.
 */

/*!
 * \qmlsignal void QtWaylandCompositor::IviApplication::iviSurfaceCreated(IviSurface *iviSurface)
 *
 * This signal is emitted when an IviSurface has been created. The supplied \a iviSurface is
 * most commonly used to instantiate a ShellSurfaceItem.
 */

/*!
 * \fn void WaylandIviApplication::iviSurfaceCreated(WaylandIviSurface *iviSurface)
 *
 * This signal is emitted when an IviSurface, \a iviSurface, has been created.
 */

WaylandIviApplicationPrivate::WaylandIviApplicationPrivate()
{
}

void WaylandIviApplicationPrivate::unregisterIviSurface(WaylandIviSurface *iviSurface)
{
    m_iviSurfaces.remove(iviSurface->iviId());
}

void WaylandIviApplicationPrivate::ivi_application_surface_create(PrivateServer::ivi_application::Resource *resource,
                                                                   uint32_t ivi_id, wl_resource *surfaceResource, uint32_t id)
{
    Q_Q(WaylandIviApplication);
    WaylandSurface *surface = WaylandSurface::fromResource(surfaceResource);

    if (m_iviSurfaces.contains(ivi_id)) {
        wl_resource_post_error(resource->handle, IVI_APPLICATION_ERROR_IVI_ID,
                               "Given ivi_id, %d, is already assigned to wl_surface@%d", ivi_id,
                               wl_resource_get_id(m_iviSurfaces[ivi_id]->surface()->resource()));
        return;
    }

    if (!surface->setRole(WaylandIviSurface::role(), resource->handle, IVI_APPLICATION_ERROR_ROLE))
        return;

    WaylandResource iviSurfaceResource(wl_resource_create(resource->client(), &ivi_surface_interface,
                                                           wl_resource_get_version(resource->handle), id));

    emit q->iviSurfaceRequested(surface, ivi_id, iviSurfaceResource);

    WaylandIviSurface *iviSurface = WaylandIviSurface::fromResource(iviSurfaceResource.resource());

    if (!iviSurface)
        iviSurface = new WaylandIviSurface(q, surface, ivi_id, iviSurfaceResource);

    m_iviSurfaces.insert(ivi_id, iviSurface);

    emit q->iviSurfaceCreated(iviSurface);
}

} // namespace Compositor

} // namespace Aurora
