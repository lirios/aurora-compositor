// SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QQmlComponent>
#include <QQmlExtensionPlugin>

#include <LiriAuroraCompositor/WaylandQuickExtension>
#include <LiriAuroraCompositor/WaylandFluidDecorationManagerV1>

namespace Aurora {

namespace Compositor {

/*!
    \qmlmodule Aurora.Compositor.Liri
    \title Liri Extension
    \ingroup qmlmodules
    \brief Provides a Qt API for the Wayland protocols of the Liri project.

    \section2 Summary
    FluidDecorationManagerV1 is an extension providing integration between
    applications made with the Fluid framework and the compositor.

    FluidDecorationManagerV1 corresponds to the Wayland interface, \c zfluid_decoration_manager_v1.

    \section2 Usage
    To use this module, import it like this:
    \qml
    import Aurora.Compositor.Liri
    \endqml
*/

AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandFluidDecorationManagerV1);

class AuroraCompositorLiriPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Aurora.Compositor.Liri"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, 1, 0);

        qmlRegisterType<WaylandFluidDecorationManagerV1QuickExtension>(uri, 1, 0, "FluidDecorationManagerV1");
        qmlRegisterUncreatableType<WaylandFluidDecorationV1>(uri, 1, 0, "FluidDecorationV1", QObject::tr("Cannot create instance of FluidDecorationV1"));
    }
};

} // namespace Compositor

} // namespace Aurora

#include "auroraliriplugin.moc"
