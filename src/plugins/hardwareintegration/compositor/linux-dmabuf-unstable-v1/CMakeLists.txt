# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

qt6_add_plugin(AuroraDmabufClientBufferIntegrationPlugin
    SHARED
    CLASS_NAME WaylandDmabufClientBufferIntegrationPlugin
    MANUAL_FINALIZATION
    ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabuf.cpp ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabuf.h
    ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabufclientbufferintegration.cpp ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/linuxdmabufclientbufferintegration.h
    main.cpp
    linux-dmabuf-unstable-v1.json
)

aurora_generate_wayland_protocol_server_sources(AuroraDmabufClientBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1/../../../3rdparty/protocol/linux-dmabuf-unstable-v1.xml
)

set_target_properties(AuroraDmabufClientBufferIntegrationPlugin
    PROPERTIES OUTPUT_NAME linux-dmabuf-unstable-v1
)

target_include_directories(AuroraDmabufClientBufferIntegrationPlugin
    PRIVATE
        ../../../../hardwareintegration/compositor/linux-dmabuf-unstable-v1
)

target_compile_definitions(AuroraDmabufClientBufferIntegrationPlugin
    PRIVATE
        -DQT_EGL_NO_X11
)

target_link_libraries(AuroraDmabufClientBufferIntegrationPlugin
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

qt6_finalize_target(AuroraDmabufClientBufferIntegrationPlugin)

install(
    TARGETS AuroraDmabufClientBufferIntegrationPlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/aurora/wayland-graphics-integration-server
)
