# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

qt6_add_plugin(AuroraBrcmEglClientBufferIntegrationPlugin
    SHARED
    CLASS_NAME WaylandBrcmClientBufferIntegrationPlugin
    MANUAL_FINALIZATION
    ../../../../hardwareintegration/compositor/brcm-egl/brcmbuffer.cpp ../../../../hardwareintegration/compositor/brcm-egl/brcmbuffer.h
    ../../../../hardwareintegration/compositor/brcm-egl/brcmeglintegration.cpp ../../../../hardwareintegration/compositor/brcm-egl/brcmeglintegration.h
    main.cpp
    brcm-egl.json
)

aurora_generate_wayland_protocol_server_sources(AuroraBrcmEglClientBufferIntegrationPlugin
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/brcm-egl/../../../extensions/brcm.xml
)

target_link_libraries(AuroraBrcmEglClientBufferIntegrationPlugin
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Liri::AuroraCompositor
    PRIVATE
        Liri::AuroraCompositorPrivate
)

qt6_finalize_target(AuroraBrcmEglClientBufferIntegrationPlugin)

install(
    TARGETS AuroraBrcmEglClientBufferIntegrationPlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/aurora/wayland-graphics-integration-server
)
