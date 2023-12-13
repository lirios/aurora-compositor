# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_plugin(AuroraDmabufClientBufferIntegrationPlugin
    TYPE
        aurora/wayland-graphics-integration-server
    OUTPUT_NAME
        linux-dmabuf-unstable-v1
    SOURCES
        ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabuf.cpp ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabuf.h
        ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabufclientbufferintegration.cpp ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabufclientbufferintegration.h
        main.cpp
        linux-dmabuf-unstable-v1.json
    DEFINES
        QT_EGL_NO_X11
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1
    LIBRARIES
        PkgConfig::EGL
        PkgConfig::Libdrm
        Wayland::Server
        Qt::CorePrivate
        Qt::GuiPrivate
        Liri::AuroraCompositor
        Liri::AuroraCompositorPrivate
)

aurora_generate_wayland_protocol_server_sources(AuroraDmabufClientBufferIntegrationPlugin
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/../../../3rdparty/protocol/linux-dmabuf-unstable-v1.xml"
)

liri_finalize_plugin(AuroraDmabufClientBufferIntegrationPlugin)
