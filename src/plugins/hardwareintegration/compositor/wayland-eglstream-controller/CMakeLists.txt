# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

qt6_add_plugin(AuroraWaylandEglStreamClientBufferIntegrationPlugin
    SHARED
    CLASS_NAME WaylandEglStreamClientBufferIntegrationPlugin
    MANUAL_FINALIZATION
    ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamcontroller.cpp ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamcontroller.h
    ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamintegration.cpp ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamintegration.h
    main.cpp
    wayland-eglstream-controller.json
)

set_target_properties(AuroraWaylandEglStreamClientBufferIntegrationPlugin
    PROPERTIES OUTPUT_NAME wayland-eglstream-controller
)

aurora_generate_wayland_protocol_server_sources(AuroraWaylandEglStreamClientBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/wayland-eglstream-controller/../../../3rdparty/protocol/wl-eglstream-controller.xml
)

target_include_directories(AuroraWaylandEglStreamClientBufferIntegrationPlugin
    PRIVATE
        ../../../../hardwareintegration/compositor/wayland-eglstream-controller
)

target_compile_definitions(AuroraWaylandEglStreamClientBufferIntegrationPlugin
    PRIVATE
        -DQT_EGL_NO_X11
)

target_link_libraries(AuroraWaylandEglStreamClientBufferIntegrationPlugin
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

qt6_finalize_target(AuroraWaylandEglStreamClientBufferIntegrationPlugin)

install(
    TARGETS AuroraWaylandEglStreamClientBufferIntegrationPlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/aurora/wayland-graphics-integration-server
)
