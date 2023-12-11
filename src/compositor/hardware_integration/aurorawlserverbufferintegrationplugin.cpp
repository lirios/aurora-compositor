// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawlserverbufferintegrationplugin_p.h"

namespace Aurora {

namespace Compositor {

namespace Internal {

ServerBufferIntegrationPlugin::ServerBufferIntegrationPlugin(QObject *parent) :
    QObject(parent)
{
}

ServerBufferIntegrationPlugin::~ServerBufferIntegrationPlugin()
{
}

}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawlserverbufferintegrationplugin_p.cpp"

