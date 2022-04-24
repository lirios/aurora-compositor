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

#include "aurorawaylandviewporter_p.h"

#include <LiriAuroraCompositor/WaylandSurface>
#include <LiriAuroraCompositor/WaylandCompositor>

#include <LiriAuroraCompositor/private/aurorawaylandsurface_p.h>

namespace Aurora {

namespace Compositor {

/*!
    \class WaylandViewporter
    \inmodule AuroraCompositor
    \since 5.13
    \brief Provides an extension for surface resizing and cropping.

    The WaylandViewporter extension provides a way for clients to resize and crop surface
    contents.

    WaylandViewporter corresponds to the Wayland interface, \c wp_viewporter.
*/

/*!
    Constructs a WaylandViewporter object.
*/
WaylandViewporter::WaylandViewporter()
    : WaylandCompositorExtensionTemplate<WaylandViewporter>()
    , d_ptr(new WaylandViewporterPrivate(this))
{
}

/*!
 * Constructs a WaylandViewporter object for the provided \a compositor.
 */
WaylandViewporter::WaylandViewporter(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate<WaylandViewporter>(compositor)
    , d_ptr(new WaylandViewporterPrivate(this))
{
}

WaylandViewporter::~WaylandViewporter()
{
}

/*!
    Initializes the extension.
*/
void WaylandViewporter::initialize()
{
    Q_D(WaylandViewporter);

    WaylandCompositorExtensionTemplate::initialize();
    auto *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find WaylandCompositor when initializing WaylandViewporter";
        return;
    }
    d->init(compositor->display(), 1);
}

/*!
    Returns the Wayland interface for the WaylandViewporter.
*/
const wl_interface *WaylandViewporter::interface()
{
    return WaylandViewporterPrivate::interface();
}

WaylandViewporterPrivate::WaylandViewporterPrivate(WaylandViewporter *self)
    : WaylandCompositorExtensionPrivate(self)
{
}

void WaylandViewporterPrivate::wp_viewporter_destroy(Resource *resource)
{
    // Viewport objects are allowed ot outlive the viewporter
    wl_resource_destroy(resource->handle);
}

void WaylandViewporterPrivate::wp_viewporter_get_viewport(Resource *resource, uint id, wl_resource *surfaceResource)
{
    auto *surface = WaylandSurface::fromResource(surfaceResource);
    if (!surface) {
        qWarning() << "Couldn't find surface for viewporter";
        return;
    }

    auto *surfacePrivate = WaylandSurfacePrivate::get(surface);
    if (surfacePrivate->viewport) {
        wl_resource_post_error(resource->handle, WP_VIEWPORTER_ERROR_VIEWPORT_EXISTS,
                               "viewport already exists for surface");
        return;
    }

    surfacePrivate->viewport = new Viewport(surface, resource->client(), id);
}

WaylandViewporterPrivate::Viewport::Viewport(WaylandSurface *surface, wl_client *client, int id)
    : PrivateServer::wp_viewport(client, id, /*version*/ 1)
    , m_surface(surface)
{
    Q_ASSERT(surface);
}

WaylandViewporterPrivate::Viewport::~Viewport()
{
    if (m_surface) {
        auto *surfacePrivate = WaylandSurfacePrivate::get(m_surface);
        Q_ASSERT(surfacePrivate->viewport == this);
        surfacePrivate->viewport = nullptr;
    }
}

// This function has to be called immediately after a surface is committed, before no
// other client events have been dispatched, or we may incorrectly error out on an
// incomplete pending state. See comment below.
void WaylandViewporterPrivate::Viewport::checkCommittedState()
{
    auto *surfacePrivate = WaylandSurfacePrivate::get(m_surface);

    // We can't use the current state for destination/source when checking,
    // as that has fallbacks to the buffer size so we can't distinguish
    // between the set/unset case. We use the pending state because no other
    // requests has modified it yet.
    QSize destination = surfacePrivate->pending.destinationSize;
    QRectF source = surfacePrivate->pending.sourceGeometry;

    if (!destination.isValid() && source.size() != source.size().toSize()) {
        wl_resource_post_error(resource()->handle, error_bad_size,
                               "non-integer size (%fx%f) with unset destination",
                               source.width(), source.height());
        return;
    }

    if (m_surface->bufferSize().isValid()) {
        QRectF max = QRectF(QPointF(), m_surface->bufferSize() / m_surface->bufferScale());
        // We can't use QRectF.contains, because that would return false for values on the border
        if (max.united(source) != max) {
            wl_resource_post_error(resource()->handle, error_out_of_buffer,
                                   "source %f,%f, %fx%f extends outside attached buffer %fx%f",
                                   source.x(), source.y(), source.width(), source.height(),
                                   max.width(), max.height());
            return;
        }
    }
}


void WaylandViewporterPrivate::Viewport::wp_viewport_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void WaylandViewporterPrivate::Viewport::wp_viewport_destroy(Resource *resource)
{
    if (m_surface) {
        auto *surfacePrivate = WaylandSurfacePrivate::get(m_surface);
        surfacePrivate->pending.destinationSize = QSize();
        surfacePrivate->pending.sourceGeometry = QRectF();
    }
    wl_resource_destroy(resource->handle);
}

void WaylandViewporterPrivate::Viewport::wp_viewport_set_source(PrivateServer::wp_viewport::Resource *resource, wl_fixed_t x, wl_fixed_t y, wl_fixed_t width, wl_fixed_t height)
{
    Q_UNUSED(resource);

    if (!m_surface) {
        wl_resource_post_error(resource->handle, error_no_surface,
                               "set_source requested for destroyed surface");
        return;
    }

    QPointF position(wl_fixed_to_double(x), wl_fixed_to_double(y));
    QSizeF size(wl_fixed_to_double(width), wl_fixed_to_double(height));
    QRectF sourceGeometry(position, size);

    if (sourceGeometry == QRectF(-1, -1, -1, -1)) {
        auto *surfacePrivate = WaylandSurfacePrivate::get(m_surface);
        surfacePrivate->pending.sourceGeometry = QRectF();
        return;
    }

    if (position.x() < 0 || position.y() < 0) {
        wl_resource_post_error(resource->handle, error_bad_value,
                               "negative position in set_source");
        return;
    }

    if (!size.isValid()) {
        wl_resource_post_error(resource->handle, error_bad_value,
                               "negative size in set_source");
        return;
    }

    auto *surfacePrivate = WaylandSurfacePrivate::get(m_surface);
    surfacePrivate->pending.sourceGeometry = sourceGeometry;
}

void WaylandViewporterPrivate::Viewport::wp_viewport_set_destination(PrivateServer::wp_viewport::Resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(resource);

    if (!m_surface) {
        wl_resource_post_error(resource->handle, error_no_surface,
                               "set_destination requested for destroyed surface");
        return;
    }

    QSize destinationSize(width, height);
    if (!destinationSize.isValid() && destinationSize != QSize(-1, -1)) {
        wl_resource_post_error(resource->handle, error_bad_value,
                               "negative size in set_destination");
        return;
    }
    auto *surfacePrivate = WaylandSurfacePrivate::get(m_surface);
    surfacePrivate->pending.destinationSize = destinationSize;
}

} // namespace Compositor

} // namespace Aurora
