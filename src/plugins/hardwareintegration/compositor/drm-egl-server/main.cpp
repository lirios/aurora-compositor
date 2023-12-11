// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <LiriAuroraCompositor/private/aurorawlserverbufferintegrationplugin_p.h>
#include "drmeglserverbufferintegration.h"

namespace Aurora {

namespace Compositor {

class DrmEglServerBufferIntegrationPlugin : public Internal::ServerBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AuroraServerBufferIntegrationFactoryInterface_iid FILE "drm-egl-server.json")
public:
    Internal::ServerBufferIntegration *create(const QString&, const QStringList&) override;
};

Internal::ServerBufferIntegration *DrmEglServerBufferIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new DrmEglServerBufferIntegration();
}

} // namespace Compositor

} // namespace Aurora

#include "main.moc"
