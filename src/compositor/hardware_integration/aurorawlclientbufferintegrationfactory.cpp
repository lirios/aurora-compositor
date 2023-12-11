// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawlclientbufferintegrationfactory_p.h"
#include "aurorawlclientbufferintegrationplugin_p.h"
#include "aurorawlclientbufferintegration_p.h"
#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

namespace Aurora {

namespace Compositor {

namespace Internal {

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, qwbifiLoader,
    (AuroraClientBufferIntegrationFactoryInterface_iid, QLatin1String("/wayland-graphics-integration-server"), Qt::CaseInsensitive))

QStringList ClientBufferIntegrationFactory::keys()
{
    return qwbifiLoader->keyMap().values();
}

ClientBufferIntegration *ClientBufferIntegrationFactory::create(const QString &name, const QStringList &args)
{
    return qLoadPlugin<ClientBufferIntegration, ClientBufferIntegrationPlugin>(qwbifiLoader(), name, args);
}

}

} // namespace Compositor

} // namespace Aurora
