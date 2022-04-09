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

#ifndef QQUICKWAYLANDSURFACE_H
#define QQUICKWAYLANDSURFACE_H

#include <LiriAuroraCompositor/aurorawaylandsurface.h>
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>

struct wl_client;

QT_REQUIRE_CONFIG(wayland_compositor_quick);

namespace Aurora {

namespace Compositor {

class WaylandQuickSurfacePrivate;
class WaylandQuickCompositor;

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandQuickSurface : public WaylandSurface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandQuickSurface)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandQuickSurface)
    Q_PROPERTY(bool useTextureAlpha READ useTextureAlpha WRITE setUseTextureAlpha NOTIFY useTextureAlphaChanged)
    QML_NAMED_ELEMENT(WaylandSurface)
    QML_ADDED_IN_VERSION(1, 0)
public:
    WaylandQuickSurface();
    WaylandQuickSurface(WaylandCompositor *compositor, WaylandClient *client, quint32 id, int version);
    ~WaylandQuickSurface() override;

    bool useTextureAlpha() const;
    void setUseTextureAlpha(bool useTextureAlpha);

protected:
    WaylandQuickSurface(WaylandQuickSurfacePrivate &dptr);

Q_SIGNALS:
    void useTextureAlphaChanged();
};

} // namespace Compositor

} // namespace Aurora

#endif
