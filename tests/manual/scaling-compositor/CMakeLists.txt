# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_executable(aurora-test-scaling-compositor
    OUTPUT_NAME
        scaling-compositor
    SOURCES
        main.cpp
    RESOURCES
        qml.qrc
    LIBRARIES
        Qt::Gui
        Qt::Qml
    GUI
    NO_TARGET_INSTALLATION
)

liri_finalize_target(aurora-test-scaling-compositor)
