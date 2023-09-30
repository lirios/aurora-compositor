// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "drmintegration.h"
#include "drmintegrationplugin.h"

namespace Aurora {

namespace Platform {

DrmIntegrationPlugin::DrmIntegrationPlugin(QObject *parent)
    : DeviceIntegrationPlugin(parent)
{
}

DeviceIntegration *DrmIntegrationPlugin::create()
{
    return new DrmIntegration(this);
}

} // namespace Platform

} // namespace Aurora
