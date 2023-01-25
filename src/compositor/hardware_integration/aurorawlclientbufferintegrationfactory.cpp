// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorafactoryloader_p.h"
#include "aurorawlclientbufferintegrationfactory_p.h"
#include "aurorawlclientbufferintegrationplugin_p.h"
#include "aurorawlclientbufferintegration_p.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

namespace Aurora {

namespace Compositor {

namespace Internal {

Q_GLOBAL_STATIC_WITH_ARGS(FactoryLoader, loader,
    (AuroraClientBufferIntegrationFactoryInterface_iid, QLatin1String("aurora/wayland-graphics-integration-server"), Qt::CaseInsensitive))

QStringList ClientBufferIntegrationFactory::keys()
{
    return loader->keyMap().values();
}

ClientBufferIntegration *ClientBufferIntegrationFactory::create(const QString &name, const QStringList &args)
{
    return auroraLoadPlugin<ClientBufferIntegration, ClientBufferIntegrationPlugin>(loader(), name, args);
}

}

} // namespace Compositor

} // namespace Aurora
