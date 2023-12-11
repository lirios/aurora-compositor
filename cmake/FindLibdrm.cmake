# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

if(NOT TARGET PkgConfig::Libdrm)
    find_package(PkgConfig QUIET)
    pkg_check_modules(Libdrm libdrm IMPORTED_TARGET)
endif()
