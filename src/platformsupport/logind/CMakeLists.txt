# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_module(AuroraLogind
    DESCRIPTION
        "Qt API for logind"
    SOURCES
        defaultlogind_p_p.h
        logind.cpp logind.h logind_p.h
        logindtypes.cpp logindtypes_p.h
    DEFINES
        #QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
        QT_NO_KEYWORDS
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::DBus
    NO_CMAKE
    NO_PKGCONFIG
)

liri_finalize_module(AuroraLogind)
