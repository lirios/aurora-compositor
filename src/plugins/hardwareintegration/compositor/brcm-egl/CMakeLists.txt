# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_plugin(AuroraBrcmEglClientBufferIntegrationPlugin
    TYPE
        aurora/wayland-graphics-integration-server
    OUTPUT_NAME
        brcm-egl
    SOURCES
        ../../../../hardwareintegration/compositor/brcm-egl/brcmbuffer.cpp ../../../../hardwareintegration/compositor/brcm-egl/brcmbuffer.h
        ../../../../hardwareintegration/compositor/brcm-egl/brcmeglintegration.cpp ../../../../hardwareintegration/compositor/brcm-egl/brcmeglintegration.h
        main.cpp
        brcm-egl.json
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/brcm-egl
    LIBRARIES
        Wayland::Server
    PUBLIC_LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Liri::AuroraCompositor
        Liri::AuroraCompositorPrivate
)

aurora_generate_wayland_protocol_server_sources(AuroraBrcmEglClientBufferIntegrationPlugin
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/brcm-egl/../../../extensions/brcm.xml"
)

liri_finalize_plugin(AuroraBrcmEglClientBufferIntegrationPlugin)
