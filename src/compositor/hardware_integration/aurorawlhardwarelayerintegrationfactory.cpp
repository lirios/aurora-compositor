// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorafactoryloader_p.h"
#include "aurorawlhardwarelayerintegrationfactory_p.h"
#include "aurorawlhardwarelayerintegrationplugin_p.h"
#include "aurorawlhardwarelayerintegration_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

namespace Aurora {

namespace Compositor {

namespace Internal {

Q_GLOBAL_STATIC_WITH_ARGS(FactoryLoader, loader,
    (AuroraHardwareLayerIntegrationFactoryInterface_iid, QLatin1String("aurora/wayland-hardware-layer-integration"), Qt::CaseInsensitive))

QStringList HardwareLayerIntegrationFactory::keys()
{
    return loader->keyMap().values();
}

HardwareLayerIntegration *HardwareLayerIntegrationFactory::create(const QString &name, const QStringList &args)
{
    return auroraLoadPlugin<HardwareLayerIntegration, HardwareLayerIntegrationPlugin>(loader(), name, args);
}

}

} // namespace Compositor

} // namespace Aurora
