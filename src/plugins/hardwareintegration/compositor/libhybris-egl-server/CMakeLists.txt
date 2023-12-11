# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

qt6_add_plugin(AuroraLibHybrisEglServerBufferIntegrationPlugin
    SHARED
    CLASS_NAME LibHybrisEglServerBufferIntegrationPlugin
    MANUAL_FINALIZATION
    ../../../../hardwareintegration/compositor/libhybris-egl-server/libhybriseglserverbufferintegration.cpp ../../../../hardwareintegration/compositor/libhybris-egl-server/libhybriseglserverbufferintegration.h
    main.cpp
    libhybris-egl-server.json
)

aurora_generate_wayland_protocol_server_sources(AuroraLibHybrisEglServerBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/libhybris-egl-server/../../../extensions/libhybris-egl-server-buffer.xml
)

set_target_properties(AuroraLibHybrisEglServerBufferIntegrationPlugin
    PROPERTIES OUTPUT_NAME libhybris-egl-server
)

target_include_directories(AuroraLibHybrisEglServerBufferIntegrationPlugin
    PRIVATE
        ../../../../hardwareintegration/compositor/libhybris-egl-server
)

target_link_libraries(AuroraLibHybrisEglServerBufferIntegrationPlugin
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Liri::AuroraCompositor
    PRIVATE
        EGL::EGL
        Qt6::GuiPrivate
        Liri::AuroraCompositorPrivate
)

qt6_finalize_target(AuroraLibHybrisEglServerBufferIntegrationPlugin)

install(
    TARGETS AuroraLibHybrisEglServerBufferIntegrationPlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/aurora/wayland-graphics-integration-server
)
