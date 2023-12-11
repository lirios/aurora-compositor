// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <LiriAuroraCompositor/WaylandQuickExtension>
#include <LiriAuroraCompositor/WaylandXdgShell>
#include <LiriAuroraCompositor/WaylandXdgDecorationManagerV1>
#include <LiriAuroraCompositor/WaylandQuickXdgOutputV1>

namespace Aurora {

namespace Compositor {

AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandXdgShell)
AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandXdgDecorationManagerV1)
AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandXdgOutputManagerV1)

/*!
    \qmlmodule Aurora.Compositor.XdgShell
    \title Qt Wayland XdgShell Extension
    \ingroup qmlmodules
    \brief Provides a Qt API for the XdgShell shell extension.

    \section2 Summary
    XdgShell is a shell extension providing window system features typical to
    desktop systems.

    XdgShell corresponds to the Wayland interface, \c xdg_shell.

    \section2 Usage
    To use this module, import it like this:
    \qml
    import Aurora.Compositor.XdgShell
    \endqml
*/

class WaylandCompositorXdgShellPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Aurora.Compositor.XdgShell"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, 1, 0);

        qmlRegisterType<WaylandXdgShellQuickExtension>(uri, 1, 3, "XdgShell");
        qmlRegisterType<WaylandXdgSurface>(uri, 1, 3, "XdgSurface");
        qmlRegisterUncreatableType<WaylandXdgToplevel>(uri, 1, 3, "XdgToplevel", QObject::tr("Cannot create instance of XdgShellToplevel"));
        qmlRegisterUncreatableType<WaylandXdgPopup>(uri, 1, 3, "XdgPopup", QObject::tr("Cannot create instance of XdgShellPopup"));

        qmlRegisterType<WaylandXdgDecorationManagerV1QuickExtension>(uri, 1, 3, "XdgDecorationManagerV1");
        qmlRegisterType<WaylandXdgOutputManagerV1QuickExtension>(uri, 1, 14, "XdgOutputManagerV1");
        qmlRegisterType<WaylandQuickXdgOutputV1>(uri, 1, 14, "XdgOutputV1");
    }
};

} // namespace Compositor

} // namespace Aurora

#include "aurorawaylandcompositorxdgshellplugin.moc"
