# SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

include(ECMQtDeclareLoggingCategory)
ecm_qt_declare_logging_category(
    AuroraPlatform_SOURCES
    HEADER "auroraplatformloggingcategories.h"
    IDENTIFIER "Aurora::Platform::gLcAuroraPlatform"
    CATEGORY_NAME "aurora.platform"
    DEFAULT_SEVERITY "Info"
    DESCRIPTION "Aurora platform abstraction"
)

liri_add_module(AuroraPlatform
    DESCRIPTION
        "Platform abstraction library for Wayland compositors using Aurora"
    SOURCES
        deviceintegration.cpp deviceintegration.h
        deviceintegrationplugin.cpp deviceintegrationplugin.h
        edid.cpp edid.h
        eglconfigchooser.cpp eglconfigchooser_p.h
        eglfsdeviceintegration.cpp eglfsdeviceintegration_p.h
        inputdevice.cpp inputdevice.h
        inputmanager.cpp inputmanager.h
        keyboarddevice.cpp keyboarddevice.h keyboarddevice_p.h
        output.cpp output.h output_p.h
        pointerdevice.cpp pointerdevice.h
        session.cpp session.h
        session_noop.cpp session_noop_p.h
        touchdevice.cpp touchdevice.h
        window.cpp window.h window_p.h
        ${AuroraPlatform_SOURCES}
    LIBRARIES
        Liri::AuroraXkbCommonSupport
        Liri::AuroraXkbCommonSupportPrivate
        EGL::EGL
        PkgConfig::LibDisplayInfo
    PUBLIC_LIBRARIES
        Qt6::Core
        Qt6::Gui
        Liri::AuroraCore
)

liri_finalize_module(AuroraPlatform)
