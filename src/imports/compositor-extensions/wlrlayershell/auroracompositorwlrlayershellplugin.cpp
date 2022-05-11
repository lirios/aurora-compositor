// SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QQmlComponent>
#include <QQmlExtensionPlugin>

#include <LiriAuroraCompositor/aurorawaylandquickextension.h>
#include <LiriAuroraCompositor/WaylandWlrLayerShellV1>
#include <LiriAuroraCompositor/WaylandWlrLayerSurfaceItem>

namespace Aurora {

namespace Compositor {

AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandWlrLayerShellV1)

class AuroraCompositorWlrLayerShellPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Aurora.Compositor.WlrLayerShell"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, 1, 0);

        qmlRegisterType<WaylandWlrLayerShellV1QuickExtension>(uri, 1, 0, "WlrLayerShellV1");
        qmlRegisterUncreatableType<WaylandWlrLayerSurfaceV1>(uri, 1, 0, "WlrLayerSurfaceV1", QObject::tr("Cannot create instance of WlrLayerSurfaceV1"));
        qmlRegisterType<WaylandWlrLayerSurfaceItem>(uri, 1, 0, "WlrLayerSurfaceItem");
    }
};

} // namespace Compositor

} // namespace Aurora

#include "auroracompositorwlrlayershellplugin.moc"
