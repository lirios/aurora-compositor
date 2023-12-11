// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawlserverbufferintegrationfactory_p.h"
#include "aurorawlserverbufferintegrationplugin_p.h"
#include "aurorawlserverbufferintegration_p.h"
#include <QtCore/private/qfactoryloader_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

namespace Aurora {

namespace Compositor {

namespace Internal {

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, qwsbifLoader,
    (AuroraServerBufferIntegrationFactoryInterface_iid, QLatin1String("/wayland-graphics-integration-server"), Qt::CaseInsensitive))

QStringList ServerBufferIntegrationFactory::keys()
{
    return qwsbifLoader->keyMap().values();
}

ServerBufferIntegration *ServerBufferIntegrationFactory::create(const QString &name, const QStringList &args)
{
    return qLoadPlugin<ServerBufferIntegration, ServerBufferIntegrationPlugin>(qwsbifLoader(), name, args);
}

}

} // namespace Compositor

} // namespace Aurora
