# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

find_package(XkbCommon REQUIRED)

add_library(eglfsxkb STATIC
    eglfsxkb.cpp
    eglfsxkb.h
)
target_link_libraries(eglfsxkb
    Qt::Core
    PkgConfig::XkbCommon
)
target_include_directories(eglfsxkb INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)
