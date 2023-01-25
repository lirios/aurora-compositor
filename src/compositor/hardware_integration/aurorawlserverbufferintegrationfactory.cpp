// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorafactoryloader_p.h"
#include "aurorawlserverbufferintegrationfactory_p.h"
#include "aurorawlserverbufferintegrationplugin_p.h"
#include "aurorawlserverbufferintegration_p.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QPluginLoader>

namespace Aurora {

namespace Compositor {

namespace Internal {

Q_GLOBAL_STATIC_WITH_ARGS(FactoryLoader, loader,
    (AuroraServerBufferIntegrationFactoryInterface_iid, QLatin1String("aurora/wayland-graphics-integration-server"), Qt::CaseInsensitive))

QStringList ServerBufferIntegrationFactory::keys()
{
    return loader->keyMap().values();
}

ServerBufferIntegration *ServerBufferIntegrationFactory::create(const QString &name, const QStringList &args)
{
    return auroraLoadPlugin<ServerBufferIntegration, ServerBufferIntegrationPlugin>(loader(), name, args);
}

}

} // namespace Compositor

} // namespace Aurora
