/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPLv3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <LiriAuroraCompositor/aurorawaylandquickextension.h>

#include "xwayland.h"
#include "xwaylandmanager.h"
#include "xwaylandserver.h"
#include "xwaylandshellsurface.h"
#include "xwaylandquickshellsurfaceitem.h"

namespace Aurora {

namespace Compositor {

class AuroraCompositorXWaylandPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface/1.0")
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Aurora.Compositor.XWayland"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, 1, 0);
        qmlRegisterType<XWayland>(uri, 1, 0, "XWayland");
        qmlRegisterType<XWaylandManager>(uri, 1, 0, "XWaylandManager");
        qmlRegisterUncreatableType<XWaylandShellSurface>(uri, 1, 0, "XWaylandShellSurfaceBase",
                                                         QLatin1String("Cannot instantiate XWaylandShellSurfaceBase"));
        qmlRegisterType<XWaylandShellSurface>(uri, 1, 0, "XWaylandShellSurface");
        qmlRegisterType<XWaylandQuickShellSurfaceItem>(uri, 1, 0, "XWaylandShellSurfaceItem");
    }
};

} // namespace Compositor

} // namespace Aurora

#include "xwaylandplugin.moc"
