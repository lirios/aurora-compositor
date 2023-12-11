# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(AuroraCompositorPresentationTimeQmlModule
    URI Aurora.Compositor.PresentationTime
    CLASS_NAME WaylandCompositorPresentationTimePlugin
    TYPEINFO aurorawaylandcompositorpresentationtimeplugin.qmltypes
    NO_GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtQuick
)

target_sources(AuroraCompositorPresentationTimeQmlModule
    PRIVATE aurorawaylandcompositorpresentationtimeplugin.cpp
)

set_target_properties(AuroraCompositorPresentationTimeQmlModule
    PROPERTIES
        OUTPUT_NAME aurorawaylandcompositorpresentationtimeplugin
)

target_link_libraries(AuroraCompositorPresentationTimeQmlModule
    PRIVATE
        Qt6::Qml
        Qt6::Quick
        Liri::AuroraCompositor
        Liri::AuroraCompositorPrivate
)

ecm_finalize_qml_module(AuroraCompositorPresentationTimeQmlModule)
