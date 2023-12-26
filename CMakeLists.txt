# SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.19)

project("Aurora"
    VERSION "0.0.0"
    DESCRIPTION "Framework for developing Wayland compositors with Qt"
    HOMEPAGE_URL "https://liri.io/"
    LANGUAGES CXX C
)

## Only build on the appropriate platforms.
if(NOT (LINUX OR APPLE OR CMAKE_SYSTEM_NAME MATCHES "QNX"))
    message(NOTICE "Skipping the build as the condition \"LINUX OR APPLE OR QNX\" is not met.")
    return()
endif()

## ECM:
find_package(ECM 5.245.0 REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

## Installation directories:
include(KDEInstallDirs)

## Compiler settings:
set(KDE_SKIP_NULLPTR_WARNINGS_SETTINGS TRUE)
include(KDECompilerSettings NO_POLICY_SCOPE)

## Enable keywords such as emit:
# TODO: Replace keywords such as emit with Q_EMIT
remove_definitions(-DQT_NO_KEYWORDS)

## CMake settings:
include(KDECMakeSettings)

## QML module:
include(ECMQmlModule)

## Shared macros and functions:
if(NOT LIRI_LOCAL_ECM)
    find_package(LiriCMakeShared "2.0.99" REQUIRED NO_MODULE)
    list(APPEND CMAKE_MODULE_PATH "${LCS_MODULE_PATH}")
endif()

## Add some paths to check for CMake modules:
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

## Features:
include(features.cmake)

## Disable use of C++ API deprecated in Qt 5.15
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)

## QML import path:
if(NOT QML_IMPORT_PATH)
    set(QML_IMPORT_PATH ${CMAKE_SOURCE_DIR}/bin CACHE STRING "" FORCE)
endif()

## Add subdirectories:
add_subdirectory(src/global)
if(FEATURE_aurora_xkbcommon)
    add_subdirectory(src/platformsupport/xkbcommon)
endif()
add_subdirectory(src/compositor)
if(FEATURE_aurora_brcm)
    add_subdirectory(src/plugins/hardwareintegration/compositor/brcm-egl)
endif()
if(FEATURE_aurora_wayland_egl)
    add_subdirectory(src/plugins/hardwareintegration/compositor/wayland-egl)
    add_subdirectory(src/plugins/hardwareintegration/compositor/wayland-eglstream-controller)
endif()
if(FEATURE_aurora_dmabuf_client_buffer)
    add_subdirectory(src/plugins/hardwareintegration/compositor/linux-dmabuf-unstable-v1)
endif()
if(FEATURE_aurora_dmabuf_server_buffer)
    add_subdirectory(src/plugins/hardwareintegration/compositor/dmabuf-server)
endif()
if(FEATURE_aurora_drm_egl_server_buffer)
    add_subdirectory(drm-egl-server)
endif()
if(FEATURE_aurora_libhybris_egl_server_buffer)
    add_subdirectory(src/plugins/hardwareintegration/compositor/libhybris-egl-server)
endif()
if(FEATURE_aurora_shm_emulation_server)
#    add_subdirectory(src/plugins/hardwareintegration/compositor/shm-emulation-server)
endif()
if(FEATURE_aurora_vulkan_server_buffer)
    add_subdirectory(src/plugins/hardwareintegration/compositor/vulkan-server)
endif()
if(FEATURE_aurora_compositor_quick)
    if(TARGET AuroraCompositor)
        add_subdirectory(src/imports/compositor-extensions/ext)
        add_subdirectory(src/imports/compositor-extensions/iviapplication)
        add_subdirectory(src/imports/compositor-extensions/liri)
        add_subdirectory(src/imports/compositor-extensions/presentationtime)
        add_subdirectory(src/imports/compositor-extensions/wlrlayershell)
        add_subdirectory(src/imports/compositor-extensions/wlroots)
        add_subdirectory(src/imports/compositor-extensions/wlshell)
        add_subdirectory(src/imports/compositor-extensions/xdgshell)
        if(FEATURE_aurora_xwayland)
            add_subdirectory(src/imports/compositor-extensions/xwayland)
        endif()
    endif()
endif()
if(FEATURE_aurora_qpa)
    add_subdirectory(src/platformheaders)
#     add_subdirectory(src/platformsupport/logind)
#     add_subdirectory(src/platformsupport/udev)
#     add_subdirectory(src/platformsupport/libinput)
#     add_subdirectory(src/platformsupport/edid)
#     add_subdirectory(src/platformsupport/kmsconvenience)
#     add_subdirectory(src/plugins/platforms/eglfs)
#     add_subdirectory(src/plugins/platforms/eglfs/deviceintegration/eglfs_kms)
#     #add_subdirectory(src/plugins/platforms/eglfs/deviceintegration/eglfs_kms_egldevice)
#     add_subdirectory(src/plugins/platforms/eglfs/deviceintegration/eglfs_kms_support)
#     if(FEATURE_aurora_qpa_x11)
#         add_subdirectory(src/plugins/platforms/eglfs/deviceintegration/eglfs_x11)
#     endif()
endif()
if(BUILD_TESTING)
    if(TARGET AuroraCompositor)
         add_subdirectory(tests/auto/compositor/compositor)
         add_subdirectory(tests/manual/qmlclient)
         add_subdirectory(tests/manual/qml-compositor)
         add_subdirectory(tests/manual/scaling-compositor)
         add_subdirectory(tests/manual/subsurface)
    endif()
    if(TARGET Liri::AuroraLogind)
#         add_subdirectory(tests/auto/logind)
    endif()
    if(TARGET Liri::AuroraUdev)
#         add_subdirectory(tests/auto/udev)
    endif()
    if(TARGET Liri::AuroraLibInput)
#         add_subdirectory(tests/manual/libinput)
    endif()
endif()
