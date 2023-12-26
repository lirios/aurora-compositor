// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandShellSurface>
#if LIRI_FEATURE_aurora_compositor_quick
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>
#endif

struct wl_resource;

namespace Aurora {

namespace Compositor {

class WaylandIviSurfacePrivate;
class WaylandSurface;
class WaylandIviApplication;
class WaylandSurfaceRole;
class WaylandResource;

class LIRIAURORACOMPOSITOR_EXPORT WaylandIviSurface : public WaylandShellSurfaceTemplate<WaylandIviSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandIviSurface)
#if LIRI_FEATURE_aurora_compositor_quick
    AURORA_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandIviSurface)
#endif
    Q_PROPERTY(Aurora::Compositor::WaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(uint iviId READ iviId NOTIFY iviIdChanged)
    Q_MOC_INCLUDE("aurorawaylandsurface.h")

public:
    WaylandIviSurface();
    WaylandIviSurface(WaylandIviApplication *application, WaylandSurface *surface, uint iviId, const WaylandResource &resource);

    Q_INVOKABLE void initialize(WaylandIviApplication *iviApplication, WaylandSurface *surface,
                                uint iviId, const WaylandResource &resource);

    WaylandSurface *surface() const;
    uint iviId() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
    static WaylandSurfaceRole *role();
    static WaylandIviSurface *fromResource(::wl_resource *resource);

    Q_INVOKABLE void sendConfigure(const QSize &size);

#if LIRI_FEATURE_aurora_compositor_quick
    WaylandQuickShellIntegration *createIntegration(WaylandQuickShellSurfaceItem *item) override;
#endif

Q_SIGNALS:
    void surfaceChanged();
    void iviIdChanged();

private:
    void initialize() override;
};

} // namespace Compositor

} // namespace Aurora

