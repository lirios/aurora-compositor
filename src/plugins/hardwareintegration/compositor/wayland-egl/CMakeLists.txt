# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

qt6_add_plugin(AuroraWaylandEglClientBufferIntegrationPlugin
    SHARED
    CLASS_NAME WaylandEglClientBufferIntegrationPlugin
    MANUAL_FINALIZATION
    ../../../../hardwareintegration/compositor/wayland-egl/waylandeglclientbufferintegration.cpp ../../../../hardwareintegration/compositor/wayland-egl/waylandeglclientbufferintegration_p.h
    main.cpp
    wayland-egl.json
)

set_target_properties(AuroraWaylandEglClientBufferIntegrationPlugin
    PROPERTIES OUTPUT_NAME wayland-egl
)

target_include_directories(AuroraWaylandEglClientBufferIntegrationPlugin
    PRIVATE
        ../../../../hardwareintegration/compositor/wayland-egl
)

target_compile_definitions(AuroraWaylandEglClientBufferIntegrationPlugin
    PRIVATE
        -DQT_EGL_NO_X11
)

target_link_libraries(AuroraWaylandEglClientBufferIntegrationPlugin
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Liri::AuroraCompositor
    PRIVATE
        EGL::EGL
        Wayland::Egl
        Qt6::GuiPrivate
        Liri::AuroraCompositorPrivate
)

qt6_finalize_target(AuroraWaylandEglClientBufferIntegrationPlugin)

install(
    TARGETS AuroraWaylandEglClientBufferIntegrationPlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/aurora/wayland-graphics-integration-server
)
