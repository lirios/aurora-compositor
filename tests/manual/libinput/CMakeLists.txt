# SPDX-FileCopyrightText: 2022-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(tst_libinput tst_libinput.cpp)

target_link_libraries(tst_libinput
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Test
        Liri::AuroraLibInput
        Liri::AuroraLogind
)
