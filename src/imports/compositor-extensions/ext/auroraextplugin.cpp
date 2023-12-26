// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QQmlComponent>
#include <QQmlExtensionPlugin>

#include <LiriAuroraCompositor/aurorawaylandquickextension.h>
#include <LiriAuroraCompositor/WaylandExtSessionLockManagerV1>

namespace Aurora {

namespace Compositor {

AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandExtSessionLockManagerV1)

class AuroraCompositorExtPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Aurora.Compositor.Ext"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, 1, 0);

        qmlRegisterType<WaylandExtSessionLockManagerV1QuickExtension>(uri, 1, 0, "ExtSessionLockManagerV1");
        qmlRegisterUncreatableType<WaylandExtSessionLockSurfaceV1>(uri, 1, 0, "ExtSessionLockSurfaceV1", QObject::tr("Cannot create instance of ExtSessionLockSurfaceV1"));
    }
};

} // namespace Compositor

} // namespace Aurora

#include "auroraextplugin.moc"
