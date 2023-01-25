// Copyright (C) 2016 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <LiriAuroraCompositor/private/aurorawlserverbufferintegrationplugin_p.h>
#include <LiriAuroraCompositor/private/aurorawlserverbufferintegration_p.h>
#include "libhybriseglserverbufferintegration.h"

namespace Aurora {

namespace Compositor {

class LibHybrisEglServerBufferIntegrationPlugin : public Internal::ServerBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AuroraServerBufferIntegrationFactoryInterface_iid FILE "libhybris-egl-server.json")
public:
    Internal::ServerBufferIntegration *create(const QString&, const QStringList&);
};

Internal::ServerBufferIntegration *LibHybrisEglServerBufferIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new LibHybrisEglServerBufferIntegration();
}

} // namespace Compositor

} // namespace Aurora

#include "main.moc"
