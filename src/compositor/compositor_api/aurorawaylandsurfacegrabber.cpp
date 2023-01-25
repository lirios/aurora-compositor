// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandsurfacegrabber.h"

#include <LiriAuroraCompositor/aurorawaylandsurface.h>
#include <LiriAuroraCompositor/aurorawaylandcompositor.h>
#include <LiriAuroraCompositor/private/aurorawaylandsurface_p.h>

namespace Aurora {

namespace Compositor {

/*!
    \class WaylandSurfaceGrabber
    \inmodule AuroraCompositor
    \since 5.8
    \brief The WaylandSurfaceGrabber class allows to read the content of a WaylandSurface.

    Sometimes it is needed to get the contents of a surface, for example to provide a screenshot
    to the user. The WaylandSurfaceGrabber class provides a simple method to do so, without
    having to care what type of buffer backs the surface, be it shared memory, OpenGL or something
    else.
*/

/*!
    \enum WaylandSurfaceGrabber::Error

    The Error enum describes the reason for a grab failure.

    \value InvalidSurface The surface is null or otherwise not valid.
    \value NoBufferAttached The client has not attached a buffer on the surface yet.
    \value UnknownBufferType The buffer attached on the surface is of an unknown type.
    \value RendererNotReady The compositor renderer is not ready to grab the surface content.
 */

class WaylandSurfaceGrabberPrivate
{
    Q_DECLARE_PUBLIC(WaylandSurfaceGrabber)
public:
    WaylandSurfaceGrabberPrivate(WaylandSurfaceGrabber *self)
        : q_ptr(self)
    {
    }

    WaylandSurface *surface = nullptr;

private:
    WaylandSurfaceGrabber *q_ptr = nullptr;
};

/*!
 * Create a WaylandSurfaceGrabber object with the given \a surface and \a parent
 */
WaylandSurfaceGrabber::WaylandSurfaceGrabber(WaylandSurface *surface, QObject *parent)
    : QObject(parent)
    , d_ptr(new WaylandSurfaceGrabberPrivate(this))
{
    Q_D(WaylandSurfaceGrabber);
    d->surface = surface;
}

WaylandSurfaceGrabber::~WaylandSurfaceGrabber()
{
}

/*!
 * Returns the surface set on this object
 */
WaylandSurface *WaylandSurfaceGrabber::surface() const
{
    Q_D(const WaylandSurfaceGrabber);
    return d->surface;
}

/*!
 * Grab the content of the surface set on this object.
 * It may not be possible to do that immediately so the success and failed signals
 * should be used to be notified of when the grab is completed.
 */
void WaylandSurfaceGrabber::grab()
{
    Q_D(WaylandSurfaceGrabber);
    if (!d->surface) {
        emit failed(InvalidSurface);
        return;
    }

    WaylandSurfacePrivate *surf = WaylandSurfacePrivate::get(d->surface);
    WaylandBufferRef buf = surf->bufferRef;
    if (!buf.hasBuffer()) {
        emit failed(NoBufferAttached);
        return;
    }

    d->surface->compositor()->grabSurface(this, buf);
}

} // namespace Compositor

} // namespace Aurora
