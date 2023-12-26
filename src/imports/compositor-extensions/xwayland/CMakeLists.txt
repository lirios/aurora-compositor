# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(AuroraXWaylandQmlPlugin
    URI Aurora.Compositor.XWayland
    VERSION 1.0
    CLASS_NAME AuroraCompositorXWaylandPlugin
    NO_GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtQuick
)

target_sources(AuroraXWaylandQmlPlugin
    PRIVATE
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
)

target_link_libraries(AuroraXWaylandQmlPlugin
    PRIVATE
        Qt6::Qml
        Qt6::Quick
        Liri::AuroraCompositor
        ${XCB_TARGETS}
        PkgConfig::Xcursor
)

ecm_finalize_qml_module(AuroraXWaylandQmlPlugin)
