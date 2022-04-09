# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

if(NOT TARGET PkgConfig::XkbCommon)
    find_package(PkgConfig QUIET)
    pkg_check_modules(XkbCommon xkbcommon REQUIRED IMPORTED_TARGET)
endif()
