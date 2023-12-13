# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_plugin(AuroraWaylandEglStreamClientBufferIntegrationPlugin
    TYPE
        aurora/wayland-graphics-integration-server
    OUTPUT_NAME
        wayland-eglstream-controller
    SOURCES
        ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamcontroller.cpp ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamcontroller.h
        ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamintegration.cpp ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamintegration.h
        main.cpp
        wayland-eglstream-controller.json
    DEFINES
        QT_EGL_NO_X11
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/wayland-eglstream-controller
    LIBRARIES
        PkgConfig::EGL
        Wayland::Egl
        Wayland::Server
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt5EglSupport::Qt5EglSupport
        Liri::AuroraCompositor
        Liri::AuroraCompositorPrivate
)

aurora_generate_wayland_protocol_server_sources(AuroraWaylandEglStreamClientBufferIntegrationPlugin
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../hardwareintegration/compositor/wayland-eglstream-controller/../../../3rdparty/protocol/wl-eglstream-controller.xml"
)

liri_finalize_plugin(AuroraWaylandEglStreamClientBufferIntegrationPlugin)
