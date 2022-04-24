# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_plugin(eglfs-kms-integration
    TYPE
        liri/egldeviceintegrations
    SOURCES
        qeglfskmsgbmcursor.cpp
        qeglfskmsgbmcursor.h
        qeglfskmsgbmdevice.cpp
        qeglfskmsgbmdevice.h
        qeglfskmsgbmintegration.cpp
        qeglfskmsgbmintegration.h
        qeglfskmsgbmmain.cpp
        qeglfskmsgbmscreen.cpp
        qeglfskmsgbmscreen.h
        qeglfskmsgbmwindow.cpp
        qeglfskmsgbmwindow.h
    DEFINES
        ${DEFINES}
    PUBLIC_DEFINES
        QT_EGL_NO_X11
    LIBRARIES
        Liri::EglFSDeviceIntegration
        Liri::EglFSDeviceIntegrationPrivate
        Liri::EglFSKmsSupport
        Liri::EglFSKmsSupportPrivate
        PkgConfig::EGL
)

liri_extend_target(eglfs-kms-integration CONDITION FEATURE_aurora_drm_atomic
    DEFINES
        EGLFS_ENABLE_DRM_ATOMIC
)

liri_finalize_plugin(eglfs-kms-integration)
