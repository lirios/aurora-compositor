// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <LiriAuroraCompositor/private/aurorawlhardwarelayerintegrationplugin_p.h>
#include "vsp2hardwarelayerintegration.h"

namespace Aurora {

namespace Compositor {

class Vsp2HardwareLayerIntegrationPlugin : public Internal::HardwareLayerIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID AuroraHardwareLayerIntegrationFactoryInterface_iid FILE "vsp2.json")
public:
    Internal::HardwareLayerIntegration *create(const QString&, const QStringList&) override;
};

Internal::HardwareLayerIntegration *Vsp2HardwareLayerIntegrationPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    Q_UNUSED(system);
    return new Vsp2HardwareLayerIntegration();
}

} // namespace Compositor

} // namespace Aurora

#include "main.moc"
