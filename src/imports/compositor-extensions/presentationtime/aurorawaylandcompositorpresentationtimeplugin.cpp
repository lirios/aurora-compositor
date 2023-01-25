// Copyright (C) 2021 LG Electronics Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <LiriAuroraCompositor/aurorawaylandquickextension.h>
#include <LiriAuroraCompositor/private/aurorawaylandpresentationtime_p.h>

namespace Aurora {

namespace Compositor {

/*!
    \qmlmodule Aurora.Compositor.PresentationTime
    \title Qt Wayland Presentation Time Extension
    \ingroup qmlmodules
    \since 6.3
    \brief Provides tracking the timing when a frame is presented on screen.

    \section2 Summary
    The PresentationTime extension provides a way to track rendering timing
    for a surface. Client can request feedbacks associated with a surface,
    then compositor send events for the feedback with the time when the surface
    is presented on-screen.

    PresentationTime corresponds to the Wayland \c wp_presentation interface.

    \section2 Usage
    To use this module, import it like this:
    \qml
    import Aurora.Compositor.PresentationTime
    \endqml
*/

AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandPresentationTime)

class WaylandCompositorPresentationTimePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Aurora.Compositor.PresentationTime"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, 1, 0);
        qmlRegisterType<WaylandPresentationTime>(uri, 1, 0, "PresentationTime");
    }
};

} // namespace Compositor

} // namespace Aurora

#include "aurorawaylandcompositorpresentationtimeplugin.moc"
