# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

find_package(Umockdev REQUIRED)

liri_add_test(tst_qtudev
    SOURCES
        tst_udev.cpp
    DEFINES
        QT_NO_KEYWORDS
    LIBRARIES
        Liri::AuroraUdev
        PkgConfig::Umockdev
)
