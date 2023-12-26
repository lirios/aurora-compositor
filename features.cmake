# SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

## Find Aurora Scanner:
find_package(AuroraScanner REQUIRED)

## Find Wayland:
set(WAYLAND_MIN_VERSION "1.15")
find_package(Wayland "${WAYLAND_MIN_VERSION}" COMPONENTS Server REQUIRED)

## Find wayland-protocols:
find_package(WaylandProtocols 1.15 REQUIRED)

## Find Qt:
set(QT_MIN_VERSION "6.6")
find_package(Qt6 "${QT_MIN_VERSION}"
    REQUIRED
    COMPONENTS
        Core
        DBus
        Xml
        Gui
        Test
)
find_package(Qt6
    QUIET
    OPTIONAL_COMPONENTS
        OpenGL
        Qml
        Quick
)

#### Features

# datadevice
option(FEATURE_aurora_datadevice "Wayland data device" ON)
add_feature_info("Aurora::DataDevice" FEATURE_aurora_datadevice "Build support for drag and drop")
set(LIRI_FEATURE_aurora_datadevice "$<IF:${FEATURE_aurora_datadevice},1,0>")

# xkbcommon
option(FEATURE_aurora_xkbcommon "Handling of keyboard descriptions" ON)
if(FEATURE_aurora_xkbcommon)
    find_package(XkbCommon QUIET)
    if(NOT TARGET XkbCommon::XkbCommon)
        message(WARNING "You need xkbcommon for Aurora::XkbCommon")
        set(FEATURE_aurora_xkbcommon OFF)
    endif()
endif()
add_feature_info("Aurora::XkbCommon" FEATURE_aurora_xkbcommon "Build support for handling of keyboard descriptions")
set(LIRI_FEATURE_aurora_xkbcommon "$<IF:${FEATURE_aurora_xkbcommon},1,0>")

# brcm
option(FEATURE_aurora_brcm "Raspberry Pi" ON)
if(FEATURE_aurora_brcm)
    find_package(EGL QUIET)
    if(NOT TARGET EGL::EGL)
        message(WARNING "You need EGL for Aurora::Brcm")
        set(FEATURE_aurora_brcm OFF)
    endif()
endif()
add_feature_info("Aurora::Brcm" FEATURE_aurora_brcm "Build Wayland compositor with Raspberry Pi hardware integration")
if(FEATURE_aurora_brcm)
    liri_config_compile_test(egl_brcm
        LABEL
            "Broadcom EGL (Raspberry Pi)"
        LIBRARIES
            EGL::EGL
        CODE "
    #include <EGL/egl.h>
    #include <bcm_host.h>

    int main(void)
    {
        vc_dispmanx_display_open(0);
        return 0;
    }
    ")
    if(NOT TEST_egl_brcm)
        message(WARNING "You need egl-brcm for Aurora::Brcm")
        set(FEATURE_aurora_brcm OFF)
    endif()
endif()
set(LIRI_FEATURE_aurora_brcm "$<IF:${FEATURE_aurora_brcm},1,0>")

# compositor-quick
option(FEATURE_aurora_compositor_quick "QtQuick integration for Wayland compositor" ON)
if(FEATURE_aurora_compositor_quick)
    if(NOT TARGET Qt6::Quick)
        message(WARNING "You need Qt Quick for Aurora::WaylandCompositorQuick")
        set(FEATURE_aurora_compositor_quick OFF)
    endif()
endif()
add_feature_info("Aurora::WaylandCompositorQuick" FEATURE_aurora_compositor_quick "Build QtQuick integration for Wayland compositor")
set(LIRI_FEATURE_aurora_compositor_quick "$<IF:${FEATURE_aurora_compositor_quick},1,0>")

# dmabuf-client-buffer
option(FEATURE_aurora_dmabuf_client_buffer "Linux dma-buf client buffer integration" ON)
if(FEATURE_aurora_dmabuf_client_buffer)
    find_package(EGL QUIET)
    find_package(Libdrm QUIET)
    if(NOT TARGET EGL::EGL)
        message(WARNING "You need EGL for Aurora::DmabufClientBuffer")
        set(FEATURE_aurora_dmabuf_client_buffer OFF)
    endif()
    if(NOT TARGET PkgConfig::Libdrm)
        message(WARNING "You need libdrm for Aurora::DmabufClientBuffer")
        set(FEATURE_aurora_dmabuf_client_buffer OFF)
    endif()
    if(NOT TARGET Qt6::OpenGL)
        message(WARNING "You need Qt OpenGL for Aurora::DmabufClientBuffer")
        set(FEATURE_aurora_dmabuf_client_buffer OFF)
    endif()
endif()
if(FEATURE_aurora_dmabuf_client_buffer)
    liri_config_compile_test(dmabuf_client_buffer
        LABEL
            "Linux client dma-buf buffer sharing"
        LIBRARIES
            EGL::EGL
            PkgConfig::Libdrm
        CODE "
    #include <EGL/egl.h>
    #include <EGL/eglext.h>
    #include <drm_mode.h>
    #include <drm_fourcc.h>

    int main(void)
    {
        // Test if DMA BUF is supported
    #ifndef EGL_LINUX_DMA_BUF_EXT
    #  error DMA BUF Extension not available
    #endif
        // Test if DMA BUF import modifier extension is supported
    #ifndef EGL_EXT_image_dma_buf_import_modifiers
    #  error DMA BUF Import modifier extension not available
    #endif
        return 0;
    }
    ")
    if(NOT TEST_dmabuf_client_buffer)
        message(WARNING "You need EGL_LINUX_DMA_BUF_EXT for Aurora::DmabufClientBuffer")
        set(FEATURE_aurora_dmabuf_client_buffer OFF)
    endif()
endif()
add_feature_info("Aurora::DmabufClientBuffer" FEATURE_aurora_dmabuf_client_buffer "Build Linux dma-buf client buffer integration")
set(LIRI_FEATURE_aurora_dmabuf_client_buffer "$<IF:${FEATURE_aurora_dmabuf_client_buffer},1,0>")

# dmabuf-server-buffer
option(FEATURE_aurora_dmabuf_server_buffer "Linux dma-buf server buffer integration" ON)
if(FEATURE_aurora_dmabuf_server_buffer)
    find_package(EGL QUIET)
    find_package(Libdrm QUIET)
    if(NOT TARGET EGL::EGL)
        message(WARNING "You need EGL for Aurora::DmabufServerBuffer")
        set(FEATURE_aurora_dmabuf_server_buffer OFF)
    endif()
    if(NOT TARGET PkgConfig::Libdrm)
        message(WARNING "You need libdrm for Aurora::DmabufServerBuffer")
        set(FEATURE_aurora_dmabuf_server_buffer OFF)
    endif()
    if(NOT TARGET Qt6::OpenGL)
        message(WARNING "You need Qt OpenGL for Aurora::DmabufServerBuffer")
        set(FEATURE_aurora_dmabuf_server_buffer OFF)
    endif()
endif()
if(FEATURE_aurora_dmabuf_server_buffer)
    liri_config_compile_test(dmabuf_server_buffer
        LABEL
            "Linux dma-buf Buffer Sharing"
        LIBRARIES
            EGL::EGL
            PkgConfig::Libdrm
        CODE "
    #include <EGL/egl.h>
    #include <EGL/eglext.h>
    #include <drm_fourcc.h>

    int main(void)
    {
    #ifdef EGL_LINUX_DMA_BUF_EXT
        return 0;
    #else
    #  error Requires EGL_LINUX_DMA_BUF_EXT
        return 1;
    #endif
        return 0;
    }
    ")
    if(NOT TEST_dmabuf_server_buffer)
        message(WARNING "You need EGL_LINUX_DMA_BUF_EXT for Aurora::DmabufServerBuffer")
        set(FEATURE_aurora_dmabuf_server_buffer OFF)
    endif()
endif()
add_feature_info("Aurora::DmabufServerBuffer" FEATURE_aurora_dmabuf_server_buffer "Build Wayland compositor with Linux dma-buf server buffer integration")
set(LIRI_FEATURE_aurora_dmabuf_server_buffer "$<IF:${FEATURE_aurora_dmabuf_server_buffer},1,0>")

# drm-atomic
option(FEATURE_aurora_drm_atomic "DRM atomic" ON)
add_feature_info("Aurora::DrmAtomic" FEATURE_aurora_drm_atomic "Build platform plugin with DRM atomic support")
set(LIRI_FEATURE_aurora_drm_atomic "$<IF:${FEATURE_aurora_drm_atomic},1,0>")

# drm-egl-server-buffer
option(FEATURE_aurora_drm_egl_server_buffer "DRM EGL" ON)
if(FEATURE_aurora_drm_egl_server_buffer)
    find_package(EGL QUIET)
    if(NOT TARGET EGL::EGL)
        message(WARNING "You need EGL for Aurora::DrmEglServerBuffer")
        set(FEATURE_aurora_drm_egl_server_buffer OFF)
    endif()
endif()
if(FEATURE_aurora_drm_egl_server_buffer)
    liri_config_compile_test(drm_egl_server
        LABEL
            "DRM EGL Server"
        LIBRARIES
            EGL::EGL
        CODE "
    #include <EGL/egl.h>
    #include <EGL/eglext.h>

    int main(void)
    {
    #ifdef EGL_MESA_drm_image
        return 0;
    #else
    #  error Requires EGL_MESA_drm_image to be defined
        return 1;
    #endif
        return 0;
    }
    ")
    if(NOT TEST_drm_egl_server)
        message(WARNING "You need EGL_MESA_drm_image for Aurora::DmabufServerBuffer")
        set(FEATURE_aurora_drm_egl_server_buffer OFF)
    endif()
endif()
add_feature_info("Aurora::DrmEglServerBuffer" FEATURE_aurora_drm_egl_server_buffer "Build Wayland compositor with DRM EGL hardware integration")
set(LIRI_FEATURE_aurora_drm_egl_server_buffer "$<IF:${FEATURE_aurora_drm_egl_server_buffer},1,0>")

# wayland-egl
option(FEATURE_aurora_wayland_egl "Wayland EGL" ON)
if(FEATURE_aurora_wayland_egl)
    find_package(EGL QUIET)
    find_package(Wayland "${WAYLAND_MIN_VERSION}" COMPONENTS Egl QUIET)
    if(NOT TARGET EGL::EGL)
        message(WARNING "You need EGL for Aurora::WaylandEgl")
        set(FEATURE_aurora_wayland_egl OFF)
    endif()
    if(NOT TARGET Wayland::Egl)
        message(WARNING "You need libdrm for Aurora::WaylandEgl")
        set(FEATURE_aurora_wayland_egl OFF)
    endif()
    if(NOT TARGET Qt6::OpenGL)
        message(WARNING "You need Qt OpenGL for Aurora::WaylandEgl")
        set(FEATURE_aurora_wayland_egl OFF)
    endif()
endif()
add_feature_info("Aurora::WaylandEgl" FEATURE_aurora_wayland_egl "Build Wayland compositor with Wayland EGL hardware integration")
set(LIRI_FEATURE_aurora_wayland_egl "$<IF:${FEATURE_aurora_wayland_egl},1,0>")

# libhybris-egl-server-buffer
option(FEATURE_aurora_libhybris_egl_server_buffer "libhybris EGL" ON)
if(FEATURE_aurora_libhybris_egl_server_buffer)
    find_package(EGL QUIET)
    if(NOT TARGET EGL::EGL)
        message(WARNING "You need EGL for Aurora::LibhybrisEgl")
        set(FEATURE_aurora_libhybris_egl_server_buffer OFF)
    endif()
    if(NOT TARGET Qt6::OpenGL)
        message(WARNING "You need Qt OpenGL for Aurora::LibhybrisEgl")
        set(FEATURE_aurora_libhybris_egl_server_buffer OFF)
    endif()
endif()
if(FEATURE_aurora_libhybris_egl_server_buffer)
    liri_config_compile_test(libhybris_egl_server
        LABEL
            "libhybris EGL Server"
        LIBRARIES
            EGL::EGL
        CODE "
    #include <EGL/egl.h>
    #include <EGL/eglext.h>
    #include <hybris/eglplatformcommon/hybris_nativebufferext.h>

    int main(void)
    {
    #ifdef EGL_HYBRIS_native_buffer
        return 0;
    #else
    #  error Requires EGL_HYBRIS_native_buffer to be defined
        return 1;
    #endif
        return 0;
    }
    ")
    if(NOT TEST_libhybris_egl_server)
        message(WARNING "You need EGL_HYBRIS_native_buffer for Aurora::LibhybrisEgl")
        set(FEATURE_aurora_libhybris_egl_server_buffer OFF)
    endif()
endif()
add_feature_info("Aurora::LibhybrisEgl" FEATURE_aurora_libhybris_egl_server_buffer "Build Wayland compositor with libhybris EGL hardware integration")
set(LIRI_FEATURE_aurora_libhybris_egl_server_buffer "$<IF:${FEATURE_aurora_libhybris_egl_server_buffer},1,0>")

# qpa
option(FEATURE_aurora_qpa "Qt platform plugin for Wayland compositors" ON)
if(FEATURE_aurora_qpa)
    find_package(EGL QUIET)
    find_package(Libudev QUIET)
    find_package(Libinput QUIET)
    find_package(Libdrm QUIET)
    find_package(Gbm QUIET)

    if(NOT TARGET EGL::EGL)
        message(WARNING "You need EGL for Aurora::QPA")
	    set(FEATURE_aurora_qpa OFF)
    endif()
    if(NOT TARGET PkgConfig::Libudev)
        message(WARNING "You need udev for Aurora::QPA")
	    set(FEATURE_aurora_qpa OFF)
    endif()
    if(NOT TARGET PkgConfig::Libinput)
        message(WARNING "You need libinput for Aurora::QPA")
	    set(FEATURE_aurora_qpa OFF)
    endif()
    if(NOT TARGET PkgConfig::Libdrm)
        message(WARNING "You need libdrm for Aurora::QPA")
	    set(FEATURE_aurora_qpa OFF)
    endif()
    if(NOT TARGET PkgConfig::Gbm)
        message(WARNING "You need gbm for Aurora::QPA")
	    set(FEATURE_aurora_qpa OFF)
    endif()
    #if(NOT TARGET Qt${QT_MAJOR_VERSION}FontDatabaseSupport::Qt${QT_MAJOR_VERSION}FontDatabaseSupport)
    #    message(WARNING "You need Qt${QT_MAJOR_VERSION}FontDatabaseSupport for Aurora::QPA")
    #    set(FEATURE_aurora_qpa OFF)
    #endif()
    #if(NOT TARGET Qt${QT_MAJOR_VERSION}ThemeSupport::Qt${QT_MAJOR_VERSION}ThemeSupport)
    #    message(WARNING "You need Qt${QT_MAJOR_VERSION}ThemeSupport for Aurora::QPA")
    #    set(FEATURE_aurora_qpa OFF)
    #endif()
    #if(NOT TARGET Qt${QT_MAJOR_VERSION}EventDispatcherSupport::Qt${QT_MAJOR_VERSION}EventDispatcherSupport)
    #    message(WARNING "You need Qt${QT_MAJOR_VERSION}EventDispatcherSupport for Aurora::QPA")
    #    set(FEATURE_aurora_qpa OFF)
    #endif()
    #if(NOT TARGET Qt${QT_MAJOR_VERSION}EglSupport::Qt${QT_MAJOR_VERSION}EglSupport)
    #    message(WARNING "You need Qt${QT_MAJOR_VERSION}EglSupport for Aurora::QPA")
    #    set(FEATURE_aurora_qpa OFF)
    #endif()
    #if(NOT TARGET Qt${QT_MAJOR_VERSION}PlatformCompositorSupport::Qt${QT_MAJOR_VERSION}PlatformCompositorSupport)
    #    message(WARNING "You need Qt${QT_MAJOR_VERSION}PlatformCompositorSupport for Aurora::QPA")
    #    set(FEATURE_aurora_qpa OFF)
    #endif()
    #if(NOT TARGET Qt${QT_MAJOR_VERSION}ServiceSupport::Qt${QT_MAJOR_VERSION}ServiceSupport)
    #    message(WARNING "You need Qt${QT_MAJOR_VERSION}ServiceSupport for Aurora::QPA")
    #    set(FEATURE_aurora_qpa OFF)
    #endif()
    #if(NOT TARGET Qt${QT_MAJOR_VERSION}FbSupport::Qt${QT_MAJOR_VERSION}FbSupport)
    #    message(WARNING "You need Qt${QT_MAJOR_VERSION}FbSupport for Aurora::QPA")
    #    set(FEATURE_aurora_qpa OFF)
    #endif()
    if(NOT FEATURE_aurora_xkbcommon)
        message(WARNING "You need XkbCommon support for Aurora::QPA")
        set(FEATURE_aurora_qpa OFF)
    endif()
endif()
add_feature_info("Aurora::QPA" FEATURE_aurora_qpa "Build Qt platform plugin for Wayland compositors")
set(LIRI_FEATURE_aurora_qpa "$<IF:${FEATURE_aurora_qpa},1,0>")

# x11
if(FEATURE_aurora_qpa)
    option(FEATURE_aurora_qpa_x11 "Qt platform plugin for Wayland compositors: X11 support" ON)
    if(FEATURE_aurora_qpa_x11)
        find_package(X11)
        if(NOT X11_FOUND)
            message(WARNING "You need X11 for Aurora::QPA::X11")
            set(FEATURE_aurora_qpa_x11 OFF)
        endif()

        find_package(XCB COMPONENTS XCB)
        if(NOT XCB_FOUND)
            message(WARNING "You need XCB for Aurora::QPA::X11")
            set(FEATURE_aurora_qpa_x11 OFF)
        endif()
    endif()
else()
    set(FEATURE_aurora_qpa_x11 OFF)
endif()
add_feature_info("Aurora::QPA::X11" FEATURE_aurora_qpa "Build X11 support for the Qt platform plugin for Wayland compositors")
set(LIRI_FEATURE_aurora_qpa_x11 "$<IF:${FEATURE_aurora_qpa_x11},1,0>")

# shm-emulation-server
option(FEATURE_aurora_shm_emulation_server "Shm emulation server" ON)
if(FEATURE_aurora_shm_emulation_server)
    if(NOT TARGET Qt6::OpenGL)
        message(WARNING "You need Qt OpenGL for Aurora::ShmEmulationServer")
        set(FEATURE_aurora_shm_emulation_server OFF)
    endif()
endif()
add_feature_info("Aurora::ShmEmulationServer" FEATURE_aurora_shm_emulation_server "Build Wayland compositor with shm emulation server buffer integration")
set(LIRI_FEATURE_aurora_shm_emulation_server "$<IF:${FEATURE_aurora_shm_emulation_server},1,0>")

# vulkan-server-buffer
option(FEATURE_aurora_vulkan_server_buffer "Vulkan" ON)
if(FEATURE_aurora_vulkan_server_buffer)
    find_package(Vulkan QUIET)
    if(NOT TARGET Qt6::OpenGL)
        message(WARNING "You need Qt OpenGL for Aurora::VulkanServerBuffer")
        set(FEATURE_aurora_vulkan_server_buffer OFF)
    endif()
    if(NOT TARGET PkgConfig::Vulkan)
        message(WARNING "You need Vulkan for Aurora::VulkanServerBuffer")
        set(FEATURE_aurora_vulkan_server_buffer OFF)
    endif()
endif()
if(FEATURE_aurora_vulkan_server_buffer)
    liri_config_compile_test(vulkan_server_buffer
        LABEL
            "Vulkan Buffer Sharing"
        DEFINITIONS
            -DVK_USE_PLATFORM_WAYLAND_KHR=1
        LIBRARIES
            PkgConfig::Vulkan
        CODE "
    #include <vulkan/vulkan.h>

    int main(void)
    {
        VkExportMemoryAllocateInfoKHR exportAllocInfo = {};
        exportAllocInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
        exportAllocInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
        return 0;
    }
    ")
    if(NOT TEST_vulkan_server_buffer)
        message(WARNING "You need Vulkan for Aurora::VulkanServerBuffer")
        set(FEATURE_aurora_vulkan_server_buffer OFF)
    endif()
endif()
add_feature_info("Aurora::VulkanServerBuffer" FEATURE_aurora_vulkan_server_buffer "Build Wayland compositor with Vulkan-based server buffer integration")
set(LIRI_FEATURE_aurora_vulkan_server_buffer "$<IF:${FEATURE_aurora_vulkan_server_buffer},1,0>")

# xwayland
option(FEATURE_aurora_xwayland "XWayland" ON)
if(FEATURE_aurora_xwayland)
    find_package(X11 QUIET)
    find_package(XCB QUIET)
    find_package(XKB QUIET)
    find_package(Xcursor QUIET)

    if(NOT X11_FOUND)
        message(WARNING "You need X11 for Aurora::XWayland")
        set(FEATURE_aurora_xwayland OFF)
    endif()
    if(NOT XCB_FOUND)
        message(WARNING "You need XCB for Aurora::XWayland")
        set(FEATURE_aurora_xwayland OFF)
    endif()
    if(NOT XKB_FOUND)
        message(WARNING "You need XKB for Aurora::XWayland")
        set(FEATURE_aurora_xwayland OFF)
    endif()
    if(NOT TARGET PkgConfig::Xcursor)
        message(WARNING "You need Xcursor for Aurora::XWayland")
        set(FEATURE_aurora_xwayland OFF)
    endif()
endif()
add_feature_info("Aurora::XWayland" FEATURE_aurora_xwayland "Build XWayland support")
set(LIRI_FEATURE_aurora_xwayland "$<IF:${FEATURE_aurora_xwayland},1,0>")

## Summary:
if(NOT LIRI_SUPERBUILD)
    feature_summary(WHAT ENABLED_FEATURES DISABLED_FEATURES)
endif()
