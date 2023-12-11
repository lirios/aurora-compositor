# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

qt6_add_plugin(AuroraShmServerBufferIntegrationPlugin
    SHARED
    CLASS_NAME ShmServerBufferIntegrationPlugin
    MANUAL_FINALIZATION
    ../../../../hardwareintegration/compositor/shm-emulation-server/shmserverbufferintegration.cpp ../../../../hardwareintegration/compositor/shm-emulation-server/shmserverbufferintegration.h
    main.cpp
    shm-emulation-server.json
)

aurora_generate_wayland_protocol_server_sources(AuroraShmServerBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/shm-emulation-server/../../../extensions/shm-emulation-server-buffer.xml
)

set_target_properties(AuroraShmServerBufferIntegrationPlugin
    PROPERTIES OUTPUT_NAME shm-emulation-server
)

target_include_directories(AuroraShmServerBufferIntegrationPlugin
    PRIVATE
        ../../../../hardwareintegration/compositor/shm-emulation-server
)

target_link_libraries(AuroraShmServerBufferIntegrationPlugin
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Liri::AuroraCompositor
    PRIVATE
        Qt6::GuiPrivate
        Liri::AuroraCompositorPrivate
)

qt6_finalize_target(AuroraShmServerBufferIntegrationPlugin)

install(
    TARGETS AuroraShmServerBufferIntegrationPlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/aurora/wayland-graphics-integration-server
)
