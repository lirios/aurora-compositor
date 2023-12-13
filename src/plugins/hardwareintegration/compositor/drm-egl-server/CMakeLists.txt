# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_plugin(AuroraDrmEglServerBufferIntegrationPlugin
    TYPE
        aurora/wayland-graphics-integration-server
    OUTPUT_NAME
        drm-egl-server
    SOURCES
        ../../../../hardwareintegration/compositor/drm-egl-server/drmeglserverbufferintegration.cpp ../../../../hardwareintegration/compositor/drm-egl-server/drmeglserverbufferintegration.h
        main.cpp
        drm-egl-server.json
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/drm-egl-server
    LIBRARIES
        PkgConfig::EGL
        Wayland::Server
        Qt::CorePrivate
        Qt::GuiPrivate
        Liri::AuroraCompositor
        Liri::AuroraCompositorPrivate
)

aurora_generate_wayland_protocol_server_sources(AuroraDrmEglServerBufferIntegrationPlugin
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/drm-egl-server/../../../extensions/drm-egl-server-buffer.xml"
)

liri_finalize_plugin(AuroraDrmEglServerBufferIntegrationPlugin)
