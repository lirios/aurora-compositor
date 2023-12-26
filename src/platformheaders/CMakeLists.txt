# SPDX-FileCopyrightText: 2022-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

liri_add_module(AuroraPlatformHeaders
    DESCRIPTION
        "Platform-specific functionality"
    SOURCES
        lirieglfsfunctions.cpp lirieglfsfunctions.h
    CLASS_HEADERS
        LiriEglFSFunctions:lirieglfsfunctions.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    PUBLIC_LIBRARIES
        Qt6::Core
        Qt6::Gui
    PKGCONFIG_DEPENDENCIES
        Qt6Core
        Qt6Gui
)

liri_finalize_module(AuroraPlatformHeaders)
