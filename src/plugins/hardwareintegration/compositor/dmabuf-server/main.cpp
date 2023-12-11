// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <LiriAuroraCompositor/private/aurorawlserverbufferintegrationplugin_p.h>
#include "dmabufserverbufferintegration.h"

namespace Aurora {

namespace Compositor {

class DmaBufServerBufferIntegrationPlugin : public Internal::ServerBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AuroraServerBufferIntegrationFactoryInterface_iid FILE "dmabuf-server.json")
public:
    Internal::ServerBufferIntegration *create(const QString&, const QStringList&) override;
};

Internal::ServerBufferIntegration *DmaBufServerBufferIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new DmaBufServerBufferIntegration();
}

} // namespace Compositor

} // namespace Aurora

#include "main.moc"
