// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "auroraplatformloggingcategories.h"
#include "deviceintegrationplugin.h"
#include "eglfsdeviceintegration_p.h"

namespace Aurora {

namespace Platform {

class EglFSDeviceIntegration
{
public:
    EglFSDeviceIntegration();
    ~EglFSDeviceIntegration();

    DeviceIntegration *deviceIntegration() const;

private:
    DeviceIntegration *m_integration = nullptr;
};

EglFSDeviceIntegration::EglFSDeviceIntegration()
{
    auto integrationKeys = DeviceIntegrationFactory::keys();
    if (!integrationKeys.isEmpty()) {
        // Prioritize either Wayland or KMS/DRM
        if (qEnvironmentVariableIsSet("WAYLAND_DISPLAY")) {
            const auto waylandKey = QStringLiteral("wayland");
            if (integrationKeys.contains(waylandKey)) {
                integrationKeys.removeOne(waylandKey);
                integrationKeys.prepend(waylandKey);
            }
        } else {
            const auto drmKey = QStringLiteral("drm");
            if (integrationKeys.contains(drmKey)) {
                integrationKeys.removeOne(drmKey);
                integrationKeys.prepend(drmKey);
            }

            const auto waylandKey = QStringLiteral("wayland");
            if (integrationKeys.contains(waylandKey))
                integrationKeys.removeOne(waylandKey);
        }

        QByteArray requested;

        // Override with an environment variable
        if (qEnvironmentVariableIsSet("AURORA_QPA_INTEGRATION"))
            requested = qgetenv("AURORA_QPA_INTEGRATION");

        if (!requested.isEmpty()) {
            const auto requestedString = QString::fromLocal8Bit(requested);
            integrationKeys.removeOne(requestedString);
            integrationKeys.prepend(requestedString);
        }

        qCDebug(gLcAuroraPlatform) << "Device integration plugin keys:" << integrationKeys;
        while (!m_integration && !integrationKeys.isEmpty()) {
            const auto key = integrationKeys.takeFirst();
            qCDebug(gLcAuroraPlatform) << "Trying to load device integration:" << key;
            m_integration = DeviceIntegrationFactory::create(key);
            if (m_integration)
                qCInfo(gLcAuroraPlatform) << "Loaded device integration" << key;
        }
    }

    if (!m_integration)
        qCFatal(gLcAuroraPlatform, "No suitable device integration found!");
}

EglFSDeviceIntegration::~EglFSDeviceIntegration()
{
    if (m_integration) {
        m_integration->deleteLater();
        m_integration = nullptr;
    }
}

DeviceIntegration *EglFSDeviceIntegration::deviceIntegration() const
{
    return m_integration;
}

Q_GLOBAL_STATIC(EglFSDeviceIntegration, eglfsDeviceIntegration)

DeviceIntegration *auroraDeviceIntegration()
{
    return eglfsDeviceIntegration()->deviceIntegration();
}

} // namespace Platform

} // namespace Aurora
