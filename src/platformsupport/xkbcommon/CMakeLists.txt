# SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

liri_add_module(AuroraXkbCommonSupport
    DESCRIPTION
        "xkbcommon shared code"
    SOURCES
        auroraxkbcommon_3rdparty.cpp
        auroraxkbcommon.cpp auroraxkbcommon_p.h
    PRIVATE_HEADERS
        auroraxkbcommon_p.h
    PUBLIC_LIBRARIES
        Qt6::Core
        Qt6::Gui
        XkbCommon::XkbCommon
    LIBRARIES
        Qt6::CorePrivate
        Qt6::GuiPrivate
)

liri_finalize_module(AuroraXkbCommonSupport)
