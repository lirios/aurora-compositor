# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

qt6_add_plugin(AuroraDmaBufServerBufferIntegrationPlugin
    SHARED
    CLASS_NAME DmaBufServerBufferIntegrationPlugin
    MANUAL_FINALIZATION
    ../../../../hardwareintegration/compositor/dmabuf-server/dmabufserverbufferintegration.cpp ../../../../hardwareintegration/compositor/dmabuf-server/dmabufserverbufferintegration.h
    main.cpp
    dmabuf-server.json
)

aurora_generate_wayland_protocol_server_sources(AuroraDmaBufServerBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/dmabuf-server/../../../extensions/qt-dmabuf-server-buffer.xml
)

set_target_properties(AuroraDmaBufServerBufferIntegrationPlugin
    PROPERTIES OUTPUT_NAME dmabuf-server
)

target_include_directories(AuroraDmaBufServerBufferIntegrationPlugin
    PRIVATE
        ../../../../hardwareintegration/compositor/dmabuf-server
)

target_link_libraries(AuroraDmaBufServerBufferIntegrationPlugin
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Liri::AuroraCompositor
    PRIVATE
        EGL::EGL
        Wayland::Egl
        PkgConfig::Libdrm
        Qt6::GuiPrivate
        Liri::AuroraCompositorPrivate
)

qt6_finalize_target(AuroraDmaBufServerBufferIntegrationPlugin)

install(
    TARGETS AuroraDmaBufServerBufferIntegrationPlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/aurora/wayland-graphics-integration-server
)
