# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_plugin(AuroraShmServerBufferIntegrationPlugin
    TYPE
        aurora/wayland-graphics-integration-server
    OUTPUT_NAME
        shm-emulation-server
    SOURCES
        ../../../../hardwareintegration/compositor/shm-emulation-server/shmserverbufferintegration.cpp ../../../../hardwareintegration/compositor/shm-emulation-server/shmserverbufferintegration.h
        main.cpp
        shm-emulation-server.json
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/shm-emulation-server
    LIBRARIES
        Wayland::Server
        Qt::CorePrivate
        Qt::GuiPrivate
        Liri::AuroraCompositor
        Liri::AuroraCompositorPrivate
)

liri_generate_wayland_protocol_server_sources(AuroraShmServerBufferIntegrationPlugin
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/shm-emulation-server/../../../extensions/shm-emulation-server-buffer.xml"
)

liri_finalize_plugin(AuroraShmServerBufferIntegrationPlugin)
