// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <LiriAuroraCompositor/private/aurorawlclientbufferintegrationfactory_p.h>
#include <LiriAuroraCompositor/private/aurorawlclientbufferintegrationplugin_p.h>
#include "waylandeglclientbufferintegration_p.h"

namespace Aurora {

namespace Compositor {

class WaylandEglClientBufferIntegrationPlugin : public Internal::ClientBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AuroraClientBufferIntegrationFactoryInterface_iid FILE "wayland-egl.json")
public:
    Internal::ClientBufferIntegration *create(const QString&, const QStringList&) override;
};

Internal::ClientBufferIntegration *WaylandEglClientBufferIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new WaylandEglClientBufferIntegration();
}

} // namespace Compositor

} // namespace Aurora

#include "main.moc"
