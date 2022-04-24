# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_module(KmsSupport
    DESCRIPTION
        "KMS shared code"
    SOURCES
        qkmsdevice.cpp
        qkmsdevice_p.h
    PRIVATE_HEADERS
        qkmsdevice_p.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        PkgConfig::Libdrm
    NO_CMAKE
    NO_PKGCONFIG
    STATIC
)

liri_extend_target(KmsSupport CONDITION FEATURE_aurora_drm_atomic
    DEFINES
        EGLFS_ENABLE_DRM_ATOMIC
)

liri_finalize_module(KmsSupport)
