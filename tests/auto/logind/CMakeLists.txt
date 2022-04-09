# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_test(tst_liri_logind
    SOURCES
        fakelogind.cpp
        fakelogind.h
        tst_logind.cpp
    LIBRARIES
        Liri::AuroraLogind
)
