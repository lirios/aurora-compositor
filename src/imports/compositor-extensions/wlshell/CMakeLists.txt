# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(AuroraCompositorWlShellQmlModule
    URI Aurora.Compositor.WlShell
    CLASS_NAME WaylandCompositorWlShellPlugin
    TYPEINFO aurorawaylandcompositorwlshellplugin.qmltypes
    NO_GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtQuick
)

target_sources(AuroraCompositorWlShellQmlModule
    PRIVATE aurorawaylandcompositorwlshellplugin.cpp
)

set_target_properties(AuroraCompositorWlShellQmlModule
    PROPERTIES
        OUTPUT_NAME aurorawaylandcompositorwlshellplugin
)

target_link_libraries(AuroraCompositorWlShellQmlModule
    PRIVATE
        Qt6::Qml
        Qt6::Quick
        Liri::AuroraCompositor
)

ecm_finalize_qml_module(AuroraCompositorWlShellQmlModule)
