// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <LiriAuroraCompositor/private/aurorawlserverbufferintegrationplugin_p.h>
#include "shmserverbufferintegration.h"

namespace Aurora {

namespace Compositor {

class ShmServerBufferIntegrationPlugin : public Internal::ServerBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AuroraServerBufferIntegrationFactoryInterface_iid FILE "shm-emulation-server.json")
public:
    Internal::ServerBufferIntegration *create(const QString&, const QStringList&) override;
};

Internal::ServerBufferIntegration *ShmServerBufferIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new ShmServerBufferIntegration();
}

} // namespace Compositor

} // namespace Aurora

#include "main.moc"
