// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>
#include <QJsonDocument>

#include "auroraplatformloggingcategories.h"
#include "deviceintegrationplugin.h"

namespace Aurora {

namespace Platform {

DeviceIntegrationPlugin::DeviceIntegrationPlugin(QObject *parent)
    : QObject(parent)
{
}

QStringList DeviceIntegrationFactory::keys(const QString &pluginPath)
{
    QStringList list;

    if (!pluginPath.isEmpty())
        QCoreApplication::addLibraryPath(pluginPath);

    const auto paths = QCoreApplication::libraryPaths();
    for (const auto &path : paths) {
        const auto absolutePath =
                QDir(path).absoluteFilePath(QStringLiteral("aurora/deviceintegration"));
        QDir dir(absolutePath);

        const auto fileNames = dir.entryList(QDir::Files);
        for (const auto &fileName : fileNames) {
            QPluginLoader loader(dir.absoluteFilePath(fileName));

            if (loader.load()) {
                const auto metaData =
                        loader.metaData().value(QLatin1String("MetaData")).toVariant().toMap();
                list += metaData.value(QStringLiteral("Keys"), QStringList()).toStringList();
            }

            loader.unload();
        }
    }

    qCDebug(gLcAuroraPlatform) << "Device integration plugin keys:" << list;
    return list;
}

DeviceIntegration *DeviceIntegrationFactory::create(const QString &name, const QString &pluginPath)
{
    if (!pluginPath.isEmpty())
        QCoreApplication::addLibraryPath(pluginPath);

    const auto paths = QCoreApplication::libraryPaths();
    for (const auto &path : paths) {
        const auto absolutePath =
                QDir(path).absoluteFilePath(QStringLiteral("aurora/deviceintegration"));
        QDir dir(absolutePath);

        const auto fileNames = dir.entryList(QDir::Files);
        for (const auto &fileName : fileNames) {
            QPluginLoader loader(dir.absoluteFilePath(fileName));

            if (loader.load()) {
                const auto metaData =
                        loader.metaData().value(QLatin1String("MetaData")).toVariant().toMap();
                const auto keys =
                        metaData.value(QStringLiteral("Keys"), QStringList()).toStringList();

                if (keys.contains(name)) {
                    auto *plugin = dynamic_cast<DeviceIntegrationPlugin *>(loader.instance());
                    if (plugin)
                        return plugin->create();
                }
            }

            loader.unload();
        }
    }

    return nullptr;
}

} // namespace Platform

} // namespace Aurora
