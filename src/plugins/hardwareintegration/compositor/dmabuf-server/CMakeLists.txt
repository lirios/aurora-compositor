# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_plugin(AuroraDmaBufServerBufferIntegrationPlugin
    TYPE
        aurora/wayland-graphics-integration-server
    OUTPUT_NAME
        dmabuf-server
    SOURCES
        ../../../../hardwareintegration/compositor/dmabuf-server/dmabufserverbufferintegration.cpp ../../../../hardwareintegration/compositor/dmabuf-server/dmabufserverbufferintegration.h
        main.cpp
        dmabuf-server.json
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/dmabuf-server
    LIBRARIES
        PkgConfig::EGL
        PkgConfig::Libdrm
        Wayland::Server
        Qt::CorePrivate
        Qt::GuiPrivate
        Liri::AuroraCompositor
        Liri::AuroraCompositorPrivate
)

liri_generate_wayland_protocol_server_sources(AuroraDmaBufServerBufferIntegrationPlugin
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/dmabuf-server/../../../extensions/qt-dmabuf-server-buffer.xml"
)

liri_finalize_plugin(AuroraDmaBufServerBufferIntegrationPlugin)
