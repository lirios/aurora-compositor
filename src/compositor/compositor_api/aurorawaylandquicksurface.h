// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/aurorawaylandsurface.h>
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>

struct wl_client;

namespace Aurora {

namespace Compositor {

class WaylandQuickSurfacePrivate;
class WaylandQuickCompositor;

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickSurface : public WaylandSurface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandQuickSurface)
    AURORA_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandQuickSurface)
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

