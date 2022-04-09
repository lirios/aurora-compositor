# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_plugin(AuroraWaylandEglClientBufferIntegrationPlugin
    TYPE
        aurora/wayland-graphics-integration-server
    OUTPUT_NAME
        wayland-egl
    SOURCES
        ../../../../hardwareintegration/compositor/wayland-egl/waylandeglclientbufferintegration.cpp ../../../../hardwareintegration/compositor/wayland-egl/waylandeglclientbufferintegration_p.h
        main.cpp
        wayland-egl.json
    DEFINES
        QT_EGL_NO_X11
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/wayland-egl
    LIBRARIES
        PkgConfig::EGL
        Wayland::Egl
        Wayland::Server
        Qt5EglSupport::Qt5EglSupport
        Qt::CorePrivate
        Qt::GuiPrivate
        Liri::AuroraCompositor
        Liri::AuroraCompositorPrivate
)

liri_finalize_plugin(AuroraWaylandEglClientBufferIntegrationPlugin)
