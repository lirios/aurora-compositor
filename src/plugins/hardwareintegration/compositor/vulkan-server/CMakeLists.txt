# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

qt6_add_plugin(AuroraVulkanServerBufferIntegrationPlugin
    SHARED
    CLASS_NAME VulkanServerBufferIntegrationPlugin
    MANUAL_FINALIZATION
    ../../../../hardwareintegration/compositor/drm-egl-server/drmeglserverbufferintegration.cpp ../../../../hardwareintegration/compositor/drm-egl-server/drmeglserverbufferintegration.h
    main.cpp
    drm-egl-server.json
)

aurora_generate_wayland_protocol_server_sources(AuroraVulkanServerBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/vulkan-server/../../../extensions/qt-vulkan-server-buffer-unstable-v1.xml
)

set_target_properties(AuroraVulkanServerBufferIntegrationPlugin
    PROPERTIES OUTPUT_NAME vulkan-server
)

target_include_directories(AuroraVulkanServerBufferIntegrationPlugin
    PRIVATE
        ../../../../hardwareintegration/compositor/vulkan-server
)

target_link_libraries(AuroraVulkanServerBufferIntegrationPlugin
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Liri::AuroraCompositor
    PRIVATE
        EGL::EGL
        Qt6::GuiPrivate
        Qt6::OpenGLPrivate
        Liri::AuroraCompositorPrivate
)

qt6_finalize_target(AuroraVulkanServerBufferIntegrationPlugin)

install(
    TARGETS AuroraVulkanServerBufferIntegrationPlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/aurora/wayland-graphics-integration-server
)
