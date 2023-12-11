# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_module(AuroraUdev
    DESCRIPTION
        "Qt style API for udev"
    SOURCES
        udev.cpp udev.h udev_p.h
        udevdevice.cpp udevdevice.h udevdevice_p.h
        udevenumerate.cpp udevenumerate.h udevenumerate_p.h
        udevmonitor.cpp udevmonitor.h udevmonitor_p.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::DBus
        Liri::AuroraLogind
        PkgConfig::Libudev
    NO_CMAKE
    NO_PKGCONFIG
)

liri_finalize_module(AuroraUdev)
