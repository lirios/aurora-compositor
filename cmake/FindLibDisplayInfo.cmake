# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

if(NOT TARGET PkgConfig::LibDisplayInfo)
    find_package(PkgConfig QUIET)
    pkg_check_modules(LibDisplayInfo libdisplay-info IMPORTED_TARGET)
endif()
