// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <LiriAuroraCompositor/private/aurorawlclientbufferintegrationplugin_p.h>
#include "brcmeglintegration.h"

namespace Aurora {

namespace Compositor {

class WaylandBrcmClientBufferIntegrationPlugin : public Internal::ClientBufferIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AuroraClientBufferIntegrationFactoryInterface_iid FILE "brcm-egl.json")
public:
    Internal::ClientBufferIntegration *create(const QString&, const QStringList&) override;
};

Internal::ClientBufferIntegration *WaylandBrcmClientBufferIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new BrcmEglIntegration();
}

} // namespace Compositor

} // namespace Aurora

#include "main.moc"
