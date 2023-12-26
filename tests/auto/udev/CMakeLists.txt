# SPDX-FileCopyrightText: 2022-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

find_package(Umockdev REQUIRED)

add_executable(tst_qtudev tst_udev.cpp)

target_link_libraries(tst_qtudev
    PRIVATE
        Liri::AuroraUdev
        PkgConfig::Umockdev
)

add_test(NAME tst_qtudev
         COMMAND tst_qtudev)
