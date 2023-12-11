// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <LiriAuroraCompositor/private/aurorawlserverbufferintegrationplugin_p.h>
#include "vulkanserverbufferintegration.h"

namespace Aurora {

namespace Compositor {

class VulkanServerBufferIntegrationPlugin : public Internal::ServerBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AuroraServerBufferIntegrationFactoryInterface_iid FILE "vulkan-server.json")
public:
    Internal::ServerBufferIntegration *create(const QString&, const QStringList&) override;
};

Internal::ServerBufferIntegration *VulkanServerBufferIntegrationPlugin::create(const QString& key, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(key);
    return new VulkanServerBufferIntegration();
}

} // namespace Compositor

} // namespace Aurora

#include "main.moc"
