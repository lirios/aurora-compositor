// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <LiriAuroraCompositor/aurorawaylandquickextension.h>
#include <LiriAuroraCompositor/aurorawaylandiviapplication.h>
#include <LiriAuroraCompositor/aurorawaylandivisurface.h>

namespace Aurora {

namespace Compositor {

/*!
    \qmlmodule Aurora.Compositor.IviApplication
    \title Qt Wayland IviApplication Extension
    \ingroup qmlmodules
    \brief Provides a Qt API for the IviApplication shell extension.

    \section2 Summary
    IviApplication is a shell extension suitable for lightweight compositors,
    for example in In-Vehicle Infotainment (IVI) systems.

    IviApplication corresponds to the Wayland \c ivi_application interface.

    \section2 Usage
    To use this module, import it like this:
    \qml
    import Aurora.Compositor.IviApplication
    \endqml
*/

AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandIviApplication)

class WaylandCompositorIviApplicationPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Aurora.Compositor.IviApplication"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, 1, 0);
        qmlRegisterType<WaylandIviApplicationQuickExtension>(uri, 1, 0, "IviApplication");
        qmlRegisterType<WaylandIviSurface>(uri, 1, 0, "IviSurface");
    }
};

} // namespace Compositor

} // namespace Aurora

#include "aurorawaylandcompositoriviapplicationplugin.moc"
