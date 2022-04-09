# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_executable(tst_libinput
    SOURCES
        tst_libinput.cpp
    LIBRARIES
        Qt5::Core
        Qt5::Gui
        Qt5::Test
        Liri::AuroraLibInput
        Liri::AuroraLogind
    NO_TARGET_INSTALLATION
)

liri_finalize_executable(tst_libinput)
