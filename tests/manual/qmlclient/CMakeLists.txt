# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_executable(aurora-test-qmlclient
    OUTPUT_NAME
        qmlclient
    SOURCES
        main.cpp
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

liri_finalize_target(aurora-test-qmlclient)
