// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/WaylandSurface>
#include <LiriAuroraCompositor/WaylandResource>
#include <LiriAuroraCompositor/WaylandIviSurface>
#include <QtCore/QSize>

namespace Aurora {

namespace Compositor {

class WaylandIviApplicationPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandIviApplication : public WaylandCompositorExtensionTemplate<WaylandIviApplication>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandIviApplication)

public:
    WaylandIviApplication();
    WaylandIviApplication(WaylandCompositor *compositor);

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void iviSurfaceRequested(WaylandSurface *surface, uint iviId, const WaylandResource &resource);
    void iviSurfaceCreated(WaylandIviSurface *iviSurface);
};

} // namespace Compositor

} // namespace Aurora

