# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_module(EglFSKmsSupport
    DESCRIPTION
        "KMS support for EGL device integration"
    SOURCES
        qeglfskmsdevice.cpp
        qeglfskmsdevice.h
        qeglfskmseventreader.cpp qeglfskmseventreader.h
        qeglfskmshelpers.h
        qeglfskmsintegration.cpp
        qeglfskmsintegration.h
        qeglfskmsscreen.cpp
        qeglfskmsscreen.h
    INSTALL_HEADERS
        qeglfskmsdevice.h
        qeglfskmshelpers.h
        qeglfskmsintegration.h
        qeglfskmsscreen.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
        ${DEFINES}
    PUBLIC_DEFINES
        QT_EGL_NO_X11
    PUBLIC_LIBRARIES
        Qt5::Core
        Qt5::CorePrivate
        Qt5::Gui
        Qt5::GuiPrivate
        Liri::EglFSDeviceIntegration
        Liri::EglFSDeviceIntegrationPrivate
        Liri::AuroraEdidSupport
        Liri::AuroraEdidSupportPrivate
        Liri::AuroraKmsSupport
        Liri::AuroraKmsSupportPrivate
        PkgConfig::Libdrm
        PkgConfig::Gbm
        PkgConfig::EGL
    NO_CMAKE
    NO_PKGCONFIG
)

liri_extend_target(EglFSKmsSupport CONDITION FEATURE_aurora_drm_atomic
    DEFINES
        EGLFS_ENABLE_DRM_ATOMIC
)

liri_finalize_module(EglFSKmsSupport)
