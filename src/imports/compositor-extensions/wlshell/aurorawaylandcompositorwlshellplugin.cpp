// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <LiriAuroraCompositor/aurorawaylandquickextension.h>
#include <LiriAuroraCompositor/aurorawaylandwlshell.h>

namespace Aurora {

namespace Compositor {

AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandWlShell)

/*!
    \qmlmodule QtWayland.Compositor.WlShell
    \title Qt Wayland WlShell extension
    \ingroup qmlmodules
    \brief Provides a Qt API for the WlShell extension.

    \section2 Summary
    WlShell is a shell extension providing window system features typical to
    desktop systems. It is superseded by XdgShell and exists in Qt mainly
    for backwards compatibility with older applications.

    WlShell corresponds to the Wayland interface \c wl_shell.

    \section2 Usage
    To use this module, import it like this:
    \qml
    import Aurora.Compositor.WlShell
    \endqml
*/

class WaylandCompositorWlShellPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Aurora.Compositor.WlShell"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, 1, 0);
        qmlRegisterType<WaylandWlShellQuickExtension>(uri, 1, 0, "WlShell");
        qmlRegisterType<WaylandWlShellSurface>(uri, 1, 0, "WlShellSurface");
    }
};

} // namespace Compositor

} // namespace Aurora

#include "aurorawaylandcompositorwlshellplugin.moc"
