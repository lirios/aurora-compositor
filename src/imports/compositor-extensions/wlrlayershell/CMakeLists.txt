# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(AuroraCompositorWlrQmlPlugin
    URI Aurora.Compositor.WlrLayerShell
    VERSION 1.0
    CLASS_NAME AuroraCompositorWlrLayerShellPlugin
    NO_GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtQuick
)

target_sources(AuroraCompositorWlrQmlPlugin
    PRIVATE auroracompositorwlrlayershellplugin.cpp
)

target_link_libraries(AuroraCompositorWlrQmlPlugin
    PRIVATE
        Qt6::Qml
        Qt6::Quick
        Liri::AuroraCompositor
)

ecm_finalize_qml_module(AuroraCompositorWlrQmlPlugin)
