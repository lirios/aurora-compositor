# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_qml_plugin(AuroraCompositorPresentationTimeQmlPlugin
    MODULE_PATH
        "Aurora/Compositor/PresentationTime"
    VERSION
        "1.0"
    OUTPUT_NAME
        auroracompositopresentationtimeplugin
    SOURCES
        aurorawaylandcompositorpresentationtimeplugin.cpp
    QML_FILES
        qmldir
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    LIBRARIES
        Qt::Core
        Qt::Gui
        Liri::AuroraCompositor
        Liri::AuroraCompositorPrivate
)

liri_finalize_qml_plugin(AuroraCompositorPresentationTimeQmlPlugin)
