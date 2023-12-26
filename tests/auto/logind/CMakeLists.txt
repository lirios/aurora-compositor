# SPDX-FileCopyrightText: 2022-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(tst_liri_logind
    fakelogind.cpp fakelogind.h
    tst_logind.cpp
)
target_link_libraries(tst_liri_logind PRIVATE Liri::AuroraLogind)

add_test(NAME tst_liri_logind
         COMMAND tst_liri_logind)
