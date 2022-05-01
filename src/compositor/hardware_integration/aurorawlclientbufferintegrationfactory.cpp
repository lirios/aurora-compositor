/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
