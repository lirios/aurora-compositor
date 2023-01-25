// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <LiriAuroraCompositor/private/aurorawlclientbufferintegrationfactory_p.h>
#include <LiriAuroraCompositor/private/aurorawlclientbufferintegrationplugin_p.h>
#include "linuxdmabufclientbufferintegration.h"

namespace Aurora {

namespace Compositor {

class WaylandDmabufClientBufferIntegrationPlugin : public Internal::ClientBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AuroraClientBufferIntegrationFactoryInterface_iid FILE "linux-dmabuf-unstable-v1.json")
public:
    Internal::ClientBufferIntegration *create(const QString& key, const QStringList& paramList) override;
};

Internal::ClientBufferIntegration *WaylandDmabufClientBufferIntegrationPlugin::create(const QString& key, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(key);
    return new LinuxDmabufClientBufferIntegration();
}

} // namespace Compositor

} // namespace Aurora

#include "main.moc"
