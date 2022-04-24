/****************************************************************************
**
** Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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
    : WaylandSurface()
    , d_ptr(new WaylandQuickSurfacePrivate(this))
{

}
WaylandQuickSurface::WaylandQuickSurface(WaylandCompositor *compositor, WaylandClient *client, quint32 id, int version)
    : WaylandSurface()
    , d_ptr(new WaylandQuickSurfacePrivate(this))
{
    initialize(compositor, client, id, version);
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
