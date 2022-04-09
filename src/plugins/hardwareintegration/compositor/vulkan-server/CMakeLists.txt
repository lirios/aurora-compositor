# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_plugin(AuroraVulkanServerBufferIntegrationPlugin
    TYPE
        aurora/wayland-graphics-integration-server
    OUTPUT_NAME
        vulkan-server
    SOURCES
        ../../../../hardwareintegration/compositor/vulkan-server/vulkanserverbufferintegration.cpp ../../../../hardwareintegration/compositor/vulkan-server/vulkanserverbufferintegration.h
        ../../../../hardwareintegration/compositor/vulkan-server/vulkanwrapper.cpp ../../../../hardwareintegration/compositor/vulkan-server/vulkanwrapper.h
        main.cpp
        vulkan-server.json
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/vulkan-server
    LIBRARIES
        Wayland::Server
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::OpenGLPrivate
        Liri::AuroraCompositor
        Liri::AuroraCompositorPrivate
)

liri_generate_wayland_protocol_server_sources(AuroraVulkanServerBufferIntegrationPlugin
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/vulkan-server/../../../extensions/qt-vulkan-server-buffer-unstable-v1.xml"
)

liri_finalize_plugin(AuroraVulkanServerBufferIntegrationPlugin)
