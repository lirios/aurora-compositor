# SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_plugin(eglfs-x11-integration
    TYPE
        liri/egldeviceintegrations
    SOURCES
        qeglfsx11integration.cpp qeglfsx11integration.h
        qeglfsx11main.cpp
    DEFINES
        QT_EGL_NO_X11
    INCLUDE_DIRECTORIES
        ../../api
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Liri::EglFSDeviceIntegration
        Liri::EglFSDeviceIntegrationPrivate
        X11::X11
        X11::xcb
        X11::X11_xcb
        XCB::XCB
)
