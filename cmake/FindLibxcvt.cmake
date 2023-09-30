# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

if(NOT TARGET PkgConfig::Libxcvt)
    find_package(PkgConfig QUIET)
    pkg_check_modules(Libxcvt libxcvt IMPORTED_TARGET)
endif()
