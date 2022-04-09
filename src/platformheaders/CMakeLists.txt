# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_module(PlatformHeaders
    DESCRIPTION
        "Platform-specific functionality"
    SOURCES
        lirieglfsfunctions.cpp
        lirieglfsfunctions.h
    CLASS_HEADERS
        LiriEglFSFunctions:lirieglfsfunctions.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
    PKGCONFIG_DEPENDENCIES
        Qt${QT_DEFAULT_MAJOR_VERSION}Core
        Qt${QT_DEFAULT_MAJOR_VERSION}Gui
)

liri_finalize_module(PlatformHeaders)
