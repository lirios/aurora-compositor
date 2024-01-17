// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtGui/qpa/qplatformintegrationplugin.h>

#include "eglfsintegration.h"

class EglFSIntegrationPlugin : public QPlatformIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformIntegrationFactoryInterface_iid FILE "aurora-eglfs.json")
public:
    QPlatformIntegration *create(const QString &, const QStringList &) override;
};

QPlatformIntegration *EglFSIntegrationPlugin::create(const QString &system,
                                                     const QStringList &paramList)
{
    if (system.compare(QLatin1String("aurora-eglfs"), Qt::CaseInsensitive) == 0) {
        // Set options from parameters passed from the command line like this
        //  -platform aurora-eglfs-next:disable-input-handlers=true
        EglFSIntegrationOptions options;
        for (const auto &param : qAsConst(paramList)) {
            const auto paramArgs = param.split(QLatin1Char('='));
            if (paramArgs.length() == 2) {
                const auto name = paramArgs[0].trimmed().toLower();
                const auto value = paramArgs[1].trimmed();

                if (name == QStringLiteral("disable-input-handlers"))
                    options.disableInputHandlers = QVariant::fromValue(value).toBool();
            }
        }

        return new EglFSIntegration(options);
    }

    return nullptr;
}

#include "eglfsmain.moc"
