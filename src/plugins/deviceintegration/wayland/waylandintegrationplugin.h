// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <LiriAuroraPlatform/DeviceIntegrationPlugin>

namespace Aurora {

namespace Platform {

class WaylandIntegrationPlugin : public DeviceIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.liri.Aurora.DeviceIntegrationPlugin/1" FILE "wayland.json")
public:
    explicit WaylandIntegrationPlugin(QObject *parent = nullptr);

    DeviceIntegration *create() override;
};

} // namespace Platform

} // namespace Aurora
