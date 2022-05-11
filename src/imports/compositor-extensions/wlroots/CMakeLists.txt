# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

liri_add_qml_plugin(AuroraCompositorWlrootsQmlPlugin
    MODULE_PATH
        "Aurora/Compositor/Wlroots"
    VERSION
        "1.0"
    OUTPUT_NAME
        auroracompositorwlrootsplugin
    SOURCES
        auroracompositorwlrootsplugin.cpp
    QML_FILES
        qmldir
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_FOREACH
    LIBRARIES
        Qt::Core
        Qt::Gui
        Liri::AuroraCompositor
)

liri_finalize_qml_plugin(AuroraCompositorWlrootsQmlPlugin)
