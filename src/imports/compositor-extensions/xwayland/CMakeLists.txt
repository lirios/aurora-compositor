# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_qml_plugin(AuroraXWaylandQmlPlugin
    MODULE_PATH
        "Aurora/Compositor/XWayland"
    VERSION
        "1.0"
    OUTPUT_NAME
        auroraxwaylandplugin
    SOURCES
        sigwatch.cpp sigwatch.h
        sizehints.h
        xcbatom.cpp xcbatom.h
        xcbatoms.cpp xcbatoms.h
        xcbcursors.cpp xcbcursors.h
        xcbproperties.cpp xcbproperties.h
        xcbresources.cpp xcbresources.h
        xcbwindow.cpp xcbwindow.h
        xcbwrapper.cpp xcbwrapper.h
        xwayland.cpp xwayland.h
        xwaylandmanager.cpp xwaylandmanager.h
        xwaylandplugin.cpp
        xwaylandquickshellintegration.cpp xwaylandquickshellintegration.h
        xwaylandquickshellsurfaceitem.cpp xwaylandquickshellsurfaceitem.h
        xwaylandserver.cpp xwaylandserver.h
        xwaylandshellsurface.cpp xwaylandshellsurface.h
    QML_FILES
        qmldir
    DEFINES
        #QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    LIBRARIES
        Liri::AuroraCompositor
        ${XCB_TARGETS}
        PkgConfig::Xcursor
)

liri_finalize_qml_plugin(AuroraXWaylandQmlPlugin)
