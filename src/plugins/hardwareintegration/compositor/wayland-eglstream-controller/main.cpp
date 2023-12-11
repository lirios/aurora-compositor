// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <LiriAuroraCompositor/private/aurorawlclientbufferintegrationfactory_p.h>
#include <LiriAuroraCompositor/private/aurorawlclientbufferintegrationplugin_p.h>
#include "waylandeglstreamintegration.h"

namespace Aurora {

namespace Compositor {

class WaylandEglStreamClientBufferIntegrationPlugin : public Internal::ClientBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AuroraClientBufferIntegrationFactoryInterface_iid FILE "wayland-eglstream-controller.json")
public:
    Internal::ClientBufferIntegration *create(const QString& key, const QStringList& paramList) override;
};

Internal::ClientBufferIntegration *WaylandEglStreamClientBufferIntegrationPlugin::create(const QString& key, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(key);
    return new WaylandEglStreamClientBufferIntegration();
}

} // namespace Compositor

} // namespace Aurora

#include "main.moc"
