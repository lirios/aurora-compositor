# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_plugin(AuroraLibHybrisEglServerBufferIntegrationPlugin
    TYPE
        aurora/wayland-graphics-integration-server
    OUTPUT_NAME
        libhybris-egl-server
    SOURCES
        ../../../../hardwareintegration/compositor/libhybris-egl-server/libhybriseglserverbufferintegration.cpp ../../../../hardwareintegration/compositor/libhybris-egl-server/libhybriseglserverbufferintegration.h
        main.cpp
        libhybris-egl-server.json
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/libhybris-egl-server
    LIBRARIES
        PkgConfig::EGL
        Wayland::Server
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Liri::AuroraCompositor
        Liri::AuroraCompositorPrivate
)

aurora_generate_wayland_protocol_server_sources(AuroraLibHybrisEglServerBufferIntegrationPlugin
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/libhybris-egl-server/../../../extensions/libhybris-egl-server-buffer.xml"
)

liri_finalize_plugin(AuroraLibHybrisEglServerBufferIntegrationPlugin)
