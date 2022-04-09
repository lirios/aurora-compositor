#.rst:
# FindQt5EglSupport
# -------
#
# Try to find Qt5EglSupport on a Unix system.
#
# This will define the following variables:
#
# ``Qt5EglSupport_FOUND``
#     True if (the requested version of) Qt5EglSupport is available
# ``Qt5EglSupport_VERSION``
#     The version of Qt5EglSupport
# ``Qt5EglSupport_LIBRARIES``
#     This can be passed to target_link_libraries() instead of the ``Qt5EglSupport::Qt5EglSupport``
#     target
# ``Qt5EglSupport_INCLUDE_DIRS``
#     This should be passed to target_include_directories() if the target is not
#     used for linking
# ``Qt5EglSupport_DEFINITIONS``
#     This should be passed to target_compile_options() if the target is not
#     used for linking
#
# If ``Qt5EglSupport_FOUND`` is TRUE, it will also define the following imported target:
#
# ``Qt5EglSupport::Qt5EglSupport``
#     The Qt5EglSupport library
#
# In general we recommend using the imported target, as it is easier to use.
# Bear in mind, however, that if the target is in the link interface of an
# exported library, it must be made available by the package config file.

# SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kde.org>
# SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
# SPDX-FileCopyrightText: 2016 Takahiro Hashimoto <kenya888@gmail.com>
# SPDX-FileCopyrightText: 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

if(CMAKE_VERSION VERSION_LESS 2.8.12)
    message(FATAL_ERROR "CMake 2.8.12 is required by FindQt5EglSupport.cmake")
endif()
if(CMAKE_MINIMUM_REQUIRED_VERSION VERSION_LESS 2.8.12)
    message(AUTHOR_WARNING "Your project should require at least CMake 2.8.12 to use FindQt5EglSupport.cmake")
endif()

# Use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig)
pkg_check_modules(PKG_Qt5EglSupport QUIET Qt5Gui)

set(Qt5EglSupport_DEFINITIONS ${PKG_Qt5EglSupport_CFLAGS_OTHER})
set(Qt5EglSupport_VERSION ${PKG_Qt5EglSupport_VERSION})

find_path(Qt5EglSupport_INCLUDE_DIR
    NAMES
        QtEglSupport/private/qeglconvenience_p.h
    HINTS
        ${PKG_Qt5EglSupport_INCLUDEDIR}/QtEglSupport/${PKG_Qt5EglSupport_VERSION}/
)
find_library(Qt5EglSupport_LIBRARY
    NAMES
        Qt5EglSupport
    HINTS
        ${PKG_Qt5EglSupport_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qt5EglSupport
    FOUND_VAR
        Qt5EglSupport_FOUND
    REQUIRED_VARS
        Qt5EglSupport_LIBRARY
        Qt5EglSupport_INCLUDE_DIR
    VERSION_VAR
        Qt5EglSupport_VERSION
)

if(Qt5EglSupport_FOUND AND NOT TARGET Qt5EglSupport::Qt5EglSupport)
    add_library(Qt5EglSupport::Qt5EglSupport UNKNOWN IMPORTED)
    set_target_properties(Qt5EglSupport::Qt5EglSupport PROPERTIES
        IMPORTED_LOCATION "${Qt5EglSupport_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${Qt5EglSupport_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${Qt5EglSupport_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(Qt5EglSupport_LIBRARY Qt5EglSupport_INCLUDE_DIR)

# compatibility variables
set(Qt5EglSupport_LIBRARIES ${Qt5EglSupport_LIBRARY})
set(Qt5EglSupport_INCLUDE_DIRS ${Qt5EglSupport_INCLUDE_DIR})
set(Qt5EglSupport_VERSION_STRING ${Qt5EglSupport_VERSION})


include(FeatureSummary)
set_package_properties(Qt5EglSupport PROPERTIES
    URL "http://www.qt.io"
    DESCRIPTION "Qt EglSupport module."
)
