# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

message("XXX: ${XkbCommon_VERSION} ${XkbCommon_INCLUDE_DIRS} ${XkbCommon_LIBRARIES} ${XkbCommon_DEFINITIONS}")
foreach(path IN LISTS XkbCommon_INCLUDE_DIRS)
    message("XXX: [${path}]")
execute_process(COMMAND ls -al ${path} COMMAND_ECHO STDOUT ECHO_OUTPUT_VARIABLE)
endforeach()
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
        XkbCommon::XkbCommon
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
)

liri_finalize_module(AuroraXkbCommonSupport)
