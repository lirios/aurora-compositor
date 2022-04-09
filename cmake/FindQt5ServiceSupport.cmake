#.rst:
# FindQt5ServiceSupport
# -------
#
# Try to find Qt5ServiceSupport on a Unix system.
#
# This will define the following variables:
#
# ``Qt5ServiceSupport_FOUND``
#     True if (the requested version of) Qt5ServiceSupport is available
# ``Qt5ServiceSupport_VERSION``
#     The version of Qt5ServiceSupport
# ``Qt5ServiceSupport_LIBRARIES``
#     This can be passed to target_link_libraries() instead of the ``Qt5ServiceSupport::Qt5ServiceSupport``
#     target
# ``Qt5ServiceSupport_INCLUDE_DIRS``
#     This should be passed to target_include_directories() if the target is not
#     used for linking
# ``Qt5ServiceSupport_DEFINITIONS``
#     This should be passed to target_compile_options() if the target is not
#     used for linking
#
# If ``Qt5ServiceSupport_FOUND`` is TRUE, it will also define the following imported target:
#
# ``Qt5ServiceSupport::Qt5ServiceSupport``
#     The Qt5ServiceSupport library
#
# In general we recommend using the imported target, as it is easier to use.
# Bear in mind, however, that if the target is in the link interface of an
# exported library, it must be made available by the package config file.

# Copyright 2014 Alex Merry <alex.merry@kde.org>
# Copyright 2014 Martin Gräßlin <mgraesslin@kde.org>
# Copyright 2016 Takahiro Hashimoto <kenya888@gmail.com>
# Copyright 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

if(CMAKE_VERSION VERSION_LESS 2.8.12)
    message(FATAL_ERROR "CMake 2.8.12 is required by FindQt5ServiceSupport.cmake")
endif()
if(CMAKE_MINIMUM_REQUIRED_VERSION VERSION_LESS 2.8.12)
    message(AUTHOR_WARNING "Your project should require at least CMake 2.8.12 to use FindQt5ServiceSupport.cmake")
endif()

# Use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig)
pkg_check_modules(PKG_Qt5ServiceSupport QUIET Qt5Gui)

set(Qt5ServiceSupport_DEFINITIONS ${PKG_Qt5ServiceSupport_CFLAGS_OTHER})
set(Qt5ServiceSupport_VERSION ${PKG_Qt5ServiceSupport_VERSION})

find_path(Qt5ServiceSupport_INCLUDE_DIR
    NAMES
        QtServiceSupport/private/qgenericunixservices_p.h
    HINTS
        ${PKG_Qt5ServiceSupport_INCLUDEDIR}/QtServiceSupport/${PKG_Qt5ServiceSupport_VERSION}/
)
find_library(Qt5ServiceSupport_LIBRARY
    NAMES
        Qt5ServiceSupport
    HINTS
        ${PKG_Qt5ServiceSupport_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qt5ServiceSupport
    FOUND_VAR
        Qt5ServiceSupport_FOUND
    REQUIRED_VARS
        Qt5ServiceSupport_LIBRARY
        Qt5ServiceSupport_INCLUDE_DIR
    VERSION_VAR
        Qt5ServiceSupport_VERSION
)

if(Qt5ServiceSupport_FOUND AND NOT TARGET Qt5ServiceSupport::Qt5ServiceSupport)
    add_library(Qt5ServiceSupport::Qt5ServiceSupport UNKNOWN IMPORTED)
    set_target_properties(Qt5ServiceSupport::Qt5ServiceSupport PROPERTIES
        IMPORTED_LOCATION "${Qt5ServiceSupport_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${Qt5ServiceSupport_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${Qt5ServiceSupport_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(Qt5ServiceSupport_LIBRARY Qt5ServiceSupport_INCLUDE_DIR)

# compatibility variables
set(Qt5ServiceSupport_LIBRARIES ${Qt5ServiceSupport_LIBRARY})
set(Qt5ServiceSupport_INCLUDE_DIRS ${Qt5ServiceSupport_INCLUDE_DIR})
set(Qt5ServiceSupport_VERSION_STRING ${Qt5ServiceSupport_VERSION})


include(FeatureSummary)
set_package_properties(Qt5ServiceSupport PROPERTIES
    URL "http://www.qt.io"
    DESCRIPTION "Qt ServiceSupport module."
)
