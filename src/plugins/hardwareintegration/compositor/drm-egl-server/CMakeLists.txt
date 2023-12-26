# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

qt6_add_plugin(AuroraDrmEglServerBufferIntegrationPlugin
    SHARED
    CLASS_NAME DrmEglServerBufferIntegrationPlugin
    MANUAL_FINALIZATION
    ../../../../hardwareintegration/compositor/drm-egl-server/drmeglserverbufferintegration.cpp ../../../../hardwareintegration/compositor/drm-egl-server/drmeglserverbufferintegration.h
    main.cpp
    drm-egl-server.json
)

aurora_generate_wayland_protocol_server_sources(AuroraDrmEglServerBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/drm-egl-server/../../../extensions/drm-egl-server-buffer.xml
)

set_target_properties(AuroraDrmEglServerBufferIntegrationPlugin
    PROPERTIES OUTPUT_NAME drm-egl-server
)

target_include_directories(AuroraDrmEglServerBufferIntegrationPlugin
    PRIVATE
        ../../../../hardwareintegration/compositor/drm-egl-server
)

target_link_libraries(AuroraDrmEglServerBufferIntegrationPlugin
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Liri::AuroraCompositor
    PRIVATE
        EGL::EGL
        Qt6::GuiPrivate
        Liri::AuroraCompositorPrivate
)

qt6_finalize_target(AuroraDrmEglServerBufferIntegrationPlugin)

install(
    TARGETS AuroraDrmEglServerBufferIntegrationPlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/aurora/wayland-graphics-integration-server
)
