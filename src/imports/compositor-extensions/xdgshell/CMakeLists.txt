# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(AuroraCompositorXdgShellQmlModule
    URI Aurora.Compositor.XdgShell
    CLASS_NAME WaylandCompositorXdgShellPlugin
    TYPEINFO aurorawaylandcompositorxdgshellplugin.qmltypes
    NO_GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtQuick
)

target_sources(AuroraCompositorXdgShellQmlModule
    PRIVATE aurorawaylandcompositorxdgshellplugin.cpp
)

set_target_properties(AuroraCompositorXdgShellQmlModule
    PROPERTIES
        OUTPUT_NAME aurorawaylandcompositorxdgshellplugin
)

target_link_libraries(AuroraCompositorXdgShellQmlModule
    PRIVATE
        Qt6::Qml
        Qt6::Quick
        Liri::AuroraCompositor
)

ecm_finalize_qml_module(AuroraCompositorXdgShellQmlModule)
