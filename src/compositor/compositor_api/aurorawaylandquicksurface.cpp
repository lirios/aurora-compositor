// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QSGTexture>
#include <QQuickWindow>
#include <QDebug>

#include "aurorawaylandquicksurface.h"
#include "aurorawaylandquicksurface_p.h"
#include "aurorawaylandquickcompositor.h"
#include "aurorawaylandquickitem.h"
#include <LiriAuroraCompositor/aurorawaylandbufferref.h>
#include <LiriAuroraCompositor/WaylandView>
#include <LiriAuroraCompositor/private/aurorawaylandsurface_p.h>

namespace Aurora {

namespace Compositor {

WaylandQuickSurface::WaylandQuickSurface()
    : WaylandSurface(* new WaylandQuickSurfacePrivate())
{

}
WaylandQuickSurface::WaylandQuickSurface(WaylandCompositor *compositor, WaylandClient *client, quint32 id, int version)
                    : WaylandSurface(* new WaylandQuickSurfacePrivate())
{
    initialize(compositor, client, id, version);
}

WaylandQuickSurface::WaylandQuickSurface(WaylandQuickSurfacePrivate &dptr)
    : WaylandSurface(dptr)
{
}

WaylandQuickSurface::~WaylandQuickSurface()
{

}

/*!
 * \qmlproperty bool AuroraCompositor::WaylandSurface::useTextureAlpha
 *
 * This property specifies whether the surface should use texture alpha.
 */
bool WaylandQuickSurface::useTextureAlpha() const
{
    Q_D(const WaylandQuickSurface);
    return d->useTextureAlpha;
}

void WaylandQuickSurface::setUseTextureAlpha(bool useTextureAlpha)
{
    Q_D(WaylandQuickSurface);
    if (d->useTextureAlpha != useTextureAlpha) {
        d->useTextureAlpha = useTextureAlpha;
        emit useTextureAlphaChanged();
        emit configure(d->bufferRef.hasBuffer());
    }
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandquicksurface.cpp"
