# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_executable(aurora-test-subsurface
    OUTPUT_NAME
        subsurface
    SOURCES
        main.cpp
        shmwindow.cpp shmwindow.h
    RESOURCES
        qml.qrc
    LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::Quick
    GUI
    NO_TARGET_INSTALLATION
)

liri_finalize_target(aurora-test-subsurface)
