# SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

include(ECMQtDeclareLoggingCategory)
ecm_qt_declare_logging_category(
    AuroraCore_SOURCES
    HEADER "auroracoreloggingcategories.h"
    IDENTIFIER "Aurora::Core::gLcAuroraCore"
    CATEGORY_NAME "aurora.core"
    DEFAULT_SEVERITY "Info"
    DESCRIPTION "Aurora core library"
)

liri_add_module(AuroraCore
    DESCRIPTION
        "Utility library for Wayland compositors using Aurora"
    SOURCES
        colorspace.cpp colorspace.h
        cursorsource.cpp cursorsource.h
        shapecursorsource.cpp shapecursorsource.h
        xcursor.c xcursor.h
        xcursortheme.cpp xcursortheme_p.h
        ${AuroraCore_SOURCES}
    PUBLIC_LIBRARIES
        Qt6::Core
        Qt6::Gui
)

liri_finalize_module(AuroraCore)
