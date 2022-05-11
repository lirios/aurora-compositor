# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_module(AuroraEdidSupport
    DESCRIPTION
        "EDID parser for EGL device integration"
    SOURCES
        qedidparser.cpp
        qedidparser_p.h
        qedidvendortable_p.h
    PRIVATE_HEADERS
        qedidparser_p.h
        qedidvendortable_p.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
    NO_CMAKE
    NO_PKGCONFIG
    STATIC
)

liri_finalize_module(AuroraEdidSupport)
