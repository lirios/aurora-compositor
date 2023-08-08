// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "waylandintegration.h"
#include "waylandintegrationplugin.h"

namespace Aurora {

namespace Platform {

WaylandIntegrationPlugin::WaylandIntegrationPlugin(QObject *parent)
    : DeviceIntegrationPlugin(parent)
{
}

DeviceIntegration *WaylandIntegrationPlugin::create()
{
    return new WaylandIntegration(this);
}

} // namespace Platform

} // namespace Aurora
