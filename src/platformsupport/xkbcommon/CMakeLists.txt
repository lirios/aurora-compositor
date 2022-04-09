# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_module(AuroraXkbCommonSupport
    DESCRIPTION
        "xkbcommon shared code"
    SOURCES
        qxkbcommon_3rdparty.cpp
        qxkbcommon.cpp qxkbcommon_p.h
    PRIVATE_HEADERS
        qxkbcommon_p.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    LIBRARIES
        Qt5::Core
        Qt5::CorePrivate
        Qt5::Gui
        Qt5::GuiPrivate
        PkgConfig::XkbCommon
)

liri_finalize_module(AuroraXkbCommonSupport)
