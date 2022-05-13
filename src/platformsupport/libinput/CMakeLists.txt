# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_module(AuroraLibInput
    DESCRIPTION
        "Qt API for libinput"
    SOURCES
        libinputgesture.cpp libinputgesture.h
        libinputhandler.cpp libinputhandler.h libinputhandler_p.h
        libinputkeyboard.cpp libinputkeyboard.h libinputkeyboard_p.h
        libinputpointer.cpp libinputpointer.h
        libinputtouch.cpp libinputtouch.h
    PRIVATE_HEADERS
        libinputhandler_p.h
        libinputkeyboard_p.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::GuiPrivate
        Liri::AuroraLogind
        Liri::AuroraUdev
        PkgConfig::Libinput
    LIBRARIES
        eglfsxkb
        Qt::CorePrivate
        Liri::AuroraUdevPrivate
    NO_CMAKE
    NO_PKGCONFIG
)

liri_finalize_module(AuroraLibInput)
