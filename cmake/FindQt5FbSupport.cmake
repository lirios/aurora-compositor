#.rst:
# FindQt5FbSupport
# -------
#
# Try to find Qt5FbSupport on a Unix system.
#
# This will define the following variables:
#
# ``Qt5FbSupport_FOUND``
#     True if (the requested version of) Qt5FbSupport is available
# ``Qt5FbSupport_VERSION``
#     The version of Qt5FbSupport
# ``Qt5FbSupport_LIBRARIES``
#     This can be passed to target_link_libraries() instead of the ``Qt5FbSupport::Qt5FbSupport``
#     target
# ``Qt5FbSupport_INCLUDE_DIRS``
#     This should be passed to target_include_directories() if the target is not
#     used for linking
# ``Qt5FbSupport_DEFINITIONS``
#     This should be passed to target_compile_options() if the target is not
#     used for linking
#
# If ``Qt5FbSupport_FOUND`` is TRUE, it will also define the following imported target:
#
# ``Qt5FbSupport::Qt5FbSupport``
#     The Qt5FbSupport library
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
    message(FATAL_ERROR "CMake 2.8.12 is required by FindQt5FbSupport.cmake")
endif()
if(CMAKE_MINIMUM_REQUIRED_VERSION VERSION_LESS 2.8.12)
    message(AUTHOR_WARNING "Your project should require at least CMake 2.8.12 to use FindQt5FbSupport.cmake")
endif()

# Use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig)
pkg_check_modules(PKG_Qt5FbSupport QUIET Qt5Gui)

set(Qt5FbSupport_DEFINITIONS ${PKG_Qt5FbSupport_CFLAGS_OTHER})
set(Qt5FbSupport_VERSION ${PKG_Qt5FbSupport_VERSION})

find_path(Qt5FbSupport_INCLUDE_DIR
    NAMES
        QtFbSupport/private/qfbvthandler_p.h
    HINTS
        ${PKG_Qt5FbSupport_INCLUDEDIR}/QtFbSupport/${PKG_Qt5FbSupport_VERSION}/
)
find_library(Qt5FbSupport_LIBRARY
    NAMES
        Qt5FbSupport
    HINTS
        ${PKG_Qt5FbSupport_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qt5FbSupport
    FOUND_VAR
        Qt5FbSupport_FOUND
    REQUIRED_VARS
        Qt5FbSupport_LIBRARY
        Qt5FbSupport_INCLUDE_DIR
    VERSION_VAR
        Qt5FbSupport_VERSION
)

if(Qt5FbSupport_FOUND AND NOT TARGET Qt5FbSupport::Qt5FbSupport)
    add_library(Qt5FbSupport::Qt5FbSupport UNKNOWN IMPORTED)
    set_target_properties(Qt5FbSupport::Qt5FbSupport PROPERTIES
        IMPORTED_LOCATION "${Qt5FbSupport_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${Qt5FbSupport_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${Qt5FbSupport_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(Qt5FbSupport_LIBRARY Qt5FbSupport_INCLUDE_DIR)

# compatibility variables
set(Qt5FbSupport_LIBRARIES ${Qt5FbSupport_LIBRARY})
set(Qt5FbSupport_INCLUDE_DIRS ${Qt5FbSupport_INCLUDE_DIR})
set(Qt5FbSupport_VERSION_STRING ${Qt5FbSupport_VERSION})


include(FeatureSummary)
set_package_properties(Qt5FbSupport PROPERTIES
    URL "http://www.qt.io"
    DESCRIPTION "Qt FbSupport module."
)
