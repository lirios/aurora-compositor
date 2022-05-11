// SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QQmlComponent>
#include <QQmlExtensionPlugin>

#include <LiriAuroraCompositor/WaylandQuickExtension>
#include <LiriAuroraCompositor/WaylandWlrExportDmabufManagerV1>
#include <LiriAuroraCompositor/WaylandWlrForeignToplevelManagerV1>
#include <LiriAuroraCompositor/WaylandWlrOutputManagerV1>
#include <LiriAuroraCompositor/WaylandQuickWlrOutputHeadV1>
#include <LiriAuroraCompositor/WaylandWlrScreencopyManagerV1>

#include <LiriAuroraCompositor/aurorawaylandquickextension.h>
#include <LiriAuroraCompositor/WaylandWlrForeignToplevelManagerV1>

namespace Aurora {

namespace Compositor {

AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandWlrExportDmabufManagerV1)
AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandWlrForeignToplevelManagerV1)
AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandWlrForeignToplevelHandleV1)
AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandWlrOutputManagerV1)
AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandWlrScreencopyManagerV1)


class AuroraCompositorWlrootsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Aurora.Compositor.Wlroots"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, 1, 0);

        qmlRegisterType<WaylandWlrExportDmabufManagerV1QuickExtension>(uri, 1, 0, "WlrExportDmabufManagerV1");
        qmlRegisterUncreatableType<WaylandWlrExportDmabufFrameV1>(uri, 1, 0, "WlrExportDmabufFrameV1", QObject::tr("Cannot create instance of WlrExportDmabufFrameV1"));

        qmlRegisterType<WaylandWlrForeignToplevelManagerV1QuickExtension>(uri, 1, 0, "WlrForeignToplevelManagerV1");
        qmlRegisterType<WaylandWlrForeignToplevelHandleV1QuickExtension>(uri, 1, 0, "WlrForeignToplevelHandleV1");

        qmlRegisterType<WaylandWlrOutputManagerV1QuickExtension>(uri, 1, 0, "WlrOutputManagerV1");
        qmlRegisterType<WaylandQuickWlrOutputHeadV1>(uri, 1, 0, "WlrOutputHeadV1");
        qmlRegisterType<WaylandWlrOutputModeV1>(uri, 1, 0, "WlrOutputModeV1");
        qmlRegisterType<WaylandQuickWlrOutputConfigurationV1>(uri, 1, 0, "WlrOutputConfigurationV1");
        qmlRegisterUncreatableType<WaylandWlrOutputConfigurationHeadV1>(uri, 1, 0, "WlrOutputConfigurationHeadV1", QObject::tr("Cannot create instance of WlrOutputConfigurationHeadV1"));

        qmlRegisterType<WaylandWlrScreencopyManagerV1QuickExtension>(uri, 1, 0, "WlrScreencopyManagerV1");
        qmlRegisterUncreatableType<WaylandWlrScreencopyFrameV1>(uri, 1, 0, "WlrScreencopyFrameV1", QObject::tr("Cannot create instance of WlrScreencopyFrameV1"));
    }
};

} // namespace Compositor

} // namespace Aurora

#include "auroracompositorwlrootsplugin.moc"
