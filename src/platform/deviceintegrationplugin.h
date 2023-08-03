// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>

#include <LiriAuroraPlatform/liriauroraplatformglobal.h>

namespace Aurora {

namespace Platform {

class DeviceIntegration;

class LIRIAURORAPLATFORM_EXPORT DeviceIntegrationPlugin : public QObject
{
    Q_OBJECT
public:
    explicit DeviceIntegrationPlugin(QObject *parent = nullptr);

    virtual DeviceIntegration *create() = 0;
};

class LIRIAURORAPLATFORM_EXPORT DeviceIntegrationFactory
{
public:
    static QStringList keys(const QString &pluginPath = QString());
    static DeviceIntegration *create(const QString &name, const QString &pluginPath = QString());
};

} // namespace Platform

} // namespace Aurora

Q_DECLARE_INTERFACE(Aurora::Platform::DeviceIntegrationPlugin,
                    "io.liri.Aurora.DeviceIntegrationPlugin/1")
