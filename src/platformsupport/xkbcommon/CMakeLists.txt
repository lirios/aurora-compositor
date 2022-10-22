# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_module(AuroraXkbCommonSupport
    DESCRIPTION
        "xkbcommon shared code"
    SOURCES
        auroraxkbcommon_3rdparty.cpp
        auroraxkbcommon.cpp auroraxkbcommon_p.h
    PRIVATE_HEADERS
        auroraxkbcommon_p.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
    LIBRARIES
        PkgConfig::XkbCommon
        Qt::CorePrivate
        Qt::GuiPrivate
)

liri_finalize_module(AuroraXkbCommonSupport)
