# SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

# QtGui features
function(read_qt_features header hints)
    find_path(_path NAMES "${header}" HINTS ${hints})
    if(_path)
        file(READ "${_path}/${header}" _contents)
        string(REGEX MATCHALL "QT_FEATURE_([a-z]+)[ \t]+([01])" _features "${_contents}")
        foreach(_line IN LISTS _features)
            string(REPLACE " " ";" _line "${_line}")
            list(GET _line 0 _name)
            list(GET _line 1 _val)
            set(${_name} "${_val}")
        endforeach()
    endif()
endfunction()
read_qt_features("qtgui-config.h" ${Qt5Gui_INCLUDE_DIRS})

liri_add_module(AuroraCompositor
    DESCRIPTION
        "Wayland compositor library"
    SOURCES
        ../shared/aurorawaylandinputmethodeventbuilder.cpp ../shared/aurorawaylandinputmethodeventbuilder_p.h
        ../shared/aurorawaylandmimehelper.cpp ../shared/aurorawaylandmimehelper_p.h
        ../shared/aurorawaylandsharedmemoryformathelper_p.h
        compositor_api/aurorawaylandbufferref.cpp compositor_api/aurorawaylandbufferref.h
        compositor_api/aurorawaylandclient.cpp compositor_api/aurorawaylandclient.h
        compositor_api/aurorawaylandcompositor.cpp compositor_api/aurorawaylandcompositor.h compositor_api/aurorawaylandcompositor_p.h
        compositor_api/aurorawaylanddestroylistener.cpp compositor_api/aurorawaylanddestroylistener.h compositor_api/aurorawaylanddestroylistener_p.h
        compositor_api/aurorawaylanddrag.cpp compositor_api/aurorawaylanddrag.h
        compositor_api/aurorawaylandkeyboard.cpp compositor_api/aurorawaylandkeyboard.h compositor_api/aurorawaylandkeyboard_p.h
        compositor_api/aurorawaylandkeymap.cpp compositor_api/aurorawaylandkeymap.h compositor_api/aurorawaylandkeymap_p.h
        compositor_api/aurorawaylandoutput.cpp compositor_api/aurorawaylandoutput.h compositor_api/aurorawaylandoutput_p.h
        compositor_api/aurorawaylandoutputmode.cpp compositor_api/aurorawaylandoutputmode.h compositor_api/aurorawaylandoutputmode_p.h
        compositor_api/aurorawaylandpointer.cpp compositor_api/aurorawaylandpointer.h compositor_api/aurorawaylandpointer_p.h
        compositor_api/aurorawaylandresource.cpp compositor_api/aurorawaylandresource.h
        compositor_api/aurorawaylandseat.cpp compositor_api/aurorawaylandseat.h compositor_api/aurorawaylandseat_p.h
        compositor_api/aurorawaylandsurface.cpp compositor_api/aurorawaylandsurface.h compositor_api/aurorawaylandsurface_p.h
        compositor_api/aurorawaylandsurfacegrabber.cpp compositor_api/aurorawaylandsurfacegrabber.h
        compositor_api/aurorawaylandtouch.cpp compositor_api/aurorawaylandtouch.h compositor_api/aurorawaylandtouch_p.h
        compositor_api/aurorawaylandview.cpp compositor_api/aurorawaylandview.h compositor_api/aurorawaylandview_p.h
        extensions/aurorawaylandextsessionlockv1.cpp extensions/aurorawaylandextsessionlockv1.h extensions/aurorawaylandextsessionlockv1_p.h
        extensions/aurorawaylandfluiddecorationv1.cpp extensions/aurorawaylandfluiddecorationv1.h extensions/aurorawaylandfluiddecorationv1_p.h
        extensions/aurorawaylandidleinhibitv1.cpp extensions/aurorawaylandidleinhibitv1.h extensions/aurorawaylandidleinhibitv1_p.h
        extensions/aurorawaylandiviapplication.cpp extensions/aurorawaylandiviapplication.h extensions/aurorawaylandiviapplication_p.h
        extensions/aurorawaylandivisurface.cpp extensions/aurorawaylandivisurface.h extensions/aurorawaylandivisurface_p.h
        extensions/aurorawaylandqtwindowmanager.cpp extensions/aurorawaylandqtwindowmanager.h extensions/aurorawaylandqtwindowmanager_p.h
        extensions/aurorawaylandshell.cpp extensions/aurorawaylandshell.h extensions/aurorawaylandshell_p.h
        extensions/aurorawaylandshellsurface.cpp extensions/aurorawaylandshellsurface.h
        extensions/aurorawaylandtextinput.cpp extensions/aurorawaylandtextinput.h extensions/aurorawaylandtextinput_p.h
        extensions/aurorawaylandtextinputmanager.cpp extensions/aurorawaylandtextinputmanager.h extensions/aurorawaylandtextinputmanager_p.h
        extensions/aurorawaylandviewporter.cpp extensions/aurorawaylandviewporter.h extensions/aurorawaylandviewporter_p.h
        extensions/aurorawaylandwlrexportdmabufv1.cpp extensions/aurorawaylandwlrexportdmabufv1.h extensions/aurorawaylandwlrexportdmabufv1_p.h
        extensions/aurorawaylandwlrforeigntoplevelmanagementv1.cpp extensions/aurorawaylandwlrforeigntoplevelmanagementv1.h extensions/aurorawaylandwlrforeigntoplevelmanagementv1_p.h
        extensions/aurorawaylandwlrlayershellv1.cpp extensions/aurorawaylandwlrlayershellv1.h extensions/aurorawaylandwlrlayershellv1_p.h
        extensions/aurorawaylandwlrscreencopyv1.cpp extensions/aurorawaylandwlrscreencopyv1.h extensions/aurorawaylandwlrscreencopyv1_p.h
        extensions/aurorawaylandwlroutputmanagementv1.cpp extensions/aurorawaylandwlroutputmanagementv1.h extensions/aurorawaylandwlroutputmanagementv1_p.h
        extensions/aurorawaylandwlshell.cpp extensions/aurorawaylandwlshell.h extensions/aurorawaylandwlshell_p.h
        extensions/aurorawaylandxdgdecorationv1.cpp extensions/aurorawaylandxdgdecorationv1.h extensions/aurorawaylandxdgdecorationv1_p.h
        extensions/aurorawaylandxdgoutputv1.cpp extensions/aurorawaylandxdgoutputv1.h extensions/aurorawaylandxdgoutputv1_p.h
        extensions/aurorawaylandxdgshell.cpp extensions/aurorawaylandxdgshell.h extensions/aurorawaylandxdgshell_p.h
        extensions/aurorawlqtkey.cpp extensions/aurorawlqtkey_p.h
        extensions/aurorawlqttouch.cpp extensions/aurorawlqttouch_p.h
        global/aurorawaylandcompositorextension.cpp global/aurorawaylandcompositorextension.h global/aurorawaylandcompositorextension_p.h
        global/aurorawaylandutils_p.h
        global/aurorawaylandqmlinclude.h
        wayland_wrapper/aurorawlbuffermanager.cpp wayland_wrapper/aurorawlbuffermanager_p.h
        wayland_wrapper/aurorawlclientbuffer.cpp wayland_wrapper/aurorawlclientbuffer_p.h
        wayland_wrapper/aurorawldatadevice.cpp wayland_wrapper/aurorawldatadevice_p.h
        wayland_wrapper/aurorawldatadevicemanager.cpp wayland_wrapper/aurorawldatadevicemanager_p.h
        wayland_wrapper/aurorawldataoffer.cpp wayland_wrapper/aurorawldataoffer_p.h
        wayland_wrapper/aurorawldatasource.cpp wayland_wrapper/aurorawldatasource_p.h
        wayland_wrapper/aurorawlregion.cpp wayland_wrapper/aurorawlregion_p.h
        utils/aurorafactoryloader.cpp utils/aurorafactoryloader_p.h
        utils/auroraunixutils_p.h
    RESOURCES
        compositor.qrc
    INCLUDE_DIRECTORIES
        ../shared
        compositor_api
        extensions
        global
        wayland_wrapper
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Wayland::Server
    LIBRARIES
        Qt::GuiPrivate
        Liri::AuroraGlobalPrivate
    PKGCONFIG_DEPENDENCIES
        Qt${QT_DEFAULT_MAJOR_VERSION}Core
        Qt${QT_DEFAULT_MAJOR_VERSION}Gui
        wayland-server
)

set(_global_header_features "#include <LiriAuroraGlobalPrivate/aurorafeatures.h>
#include <QtGui/qtgui-config.h>
#include <QtGui/qtguiglobal.h>")
liri_extend_target(AuroraCompositor
    GLOBAL_HEADER_CONTENT
        "${_global_header_features}"
)

aurora_generate_wayland_protocol_server_sources(AuroraCompositor
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/ext-session-lock-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/idle-inhibit-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/ivi-application.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/presentation-time.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/scaler.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/text-input-unstable-v2.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/text-input-unstable-v4-wip.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/viewporter.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/wayland.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/wlr-export-dmabuf-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/wlr-foreign-toplevel-management-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/wlr-layer-shell-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/wlr-output-management-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/wlr-screencopy-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/xdg-decoration-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/xdg-output-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/protocol/xdg-shell.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/fluid-decoration-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/hardware-integration.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/qt-key-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/qt-text-input-method-unstable-v1.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/qt-windowmanager.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/server-buffer-extension.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../extensions/touch-extension.xml
)

liri_extend_target(AuroraCompositor CONDITION FEATURE_aurora_xkbcommon
    PUBLIC_LIBRARIES
        Liri::AuroraXkbCommonSupport
        Liri::AuroraXkbCommonSupportPrivate
)

liri_extend_target(AuroraCompositor CONDITION TARGET Qt::OpenGL
    SOURCES
        hardware_integration/aurorawlclientbufferintegrationfactory.cpp hardware_integration/aurorawlclientbufferintegrationfactory_p.h
        hardware_integration/aurorawlclientbufferintegrationplugin.cpp hardware_integration/aurorawlclientbufferintegrationplugin_p.h
        hardware_integration/aurorawlhardwarelayerintegration.cpp hardware_integration/aurorawlhardwarelayerintegration_p.h
        hardware_integration/aurorawlhardwarelayerintegrationfactory.cpp hardware_integration/aurorawlhardwarelayerintegrationfactory_p.h
        hardware_integration/aurorawlhardwarelayerintegrationplugin.cpp hardware_integration/aurorawlhardwarelayerintegrationplugin_p.h
        hardware_integration/aurorawlhwintegration.cpp hardware_integration/aurorawlhwintegration_p.h
        hardware_integration/aurorawlclientbufferintegration.cpp hardware_integration/aurorawlclientbufferintegration_p.h
        hardware_integration/aurorawlserverbufferintegration.cpp hardware_integration/aurorawlserverbufferintegration_p.h
        hardware_integration/aurorawlserverbufferintegrationfactory.cpp hardware_integration/aurorawlserverbufferintegrationfactory_p.h
        hardware_integration/aurorawlserverbufferintegrationplugin.cpp hardware_integration/aurorawlserverbufferintegrationplugin_p.h
    INCLUDE_DIRECTORIES
        utils
    PUBLIC_LIBRARIES
        Qt::OpenGL
    PKGCONFIG_DEPENDENCIES
        Qt${QT_DEFAULT_MAJOR_VERSION}OpenGL
)

if(QT_DEFAULT_MAJOR_VERSION EQUAL 6)
    liri_extend_target(AuroraCompositor CONDITION QT_FEATURE_im
        SOURCES
            compositor_api/aurorawaylandinputmethodcontrol.cpp compositor_api/aurorawaylandinputmethodcontrol.h compositor_api/aurorawaylandinputmethodcontrol_p.h
            extensions/aurorawaylandqttextinputmethod.cpp extensions/aurorawaylandqttextinputmethod.h extensions/aurorawaylandqttextinputmethod_p.h
            extensions/aurorawaylandqttextinputmethodmanager.cpp extensions/aurorawaylandqttextinputmethodmanager.h extensions/aurorawaylandqttextinputmethodmanager_p.h
    )
endif()

liri_extend_target(AuroraCompositor CONDITION TARGET Qt::Quick
    SOURCES
        compositor_api/aurorawaylandmousetracker.cpp compositor_api/aurorawaylandmousetracker_p.h
        compositor_api/aurorawaylandquickchildren.h
        compositor_api/aurorawaylandquickcompositor.cpp compositor_api/aurorawaylandquickcompositor.h
        compositor_api/aurorawaylandquickitem.cpp compositor_api/aurorawaylandquickitem.h compositor_api/aurorawaylandquickitem_p.h
        compositor_api/aurorawaylandquickoutput.cpp compositor_api/aurorawaylandquickoutput.h
        compositor_api/aurorawaylandquicksurface.cpp compositor_api/aurorawaylandquicksurface.h compositor_api/aurorawaylandquicksurface_p.h
        extensions/aurorawaylandextsessionlockv1integration.cpp extensions/aurorawaylandextsessionlockv1integration_p.h
        extensions/aurorawaylandivisurfaceintegration.cpp extensions/aurorawaylandivisurfaceintegration_p.h
        extensions/aurorawaylandquickshellintegration.cpp extensions/aurorawaylandquickshellintegration.h
        extensions/aurorawaylandquickshellsurfaceitem.cpp extensions/aurorawaylandquickshellsurfaceitem.h extensions/aurorawaylandquickshellsurfaceitem_p.h
        extensions/aurorawaylandquickwlroutputmanagementv1.cpp extensions/aurorawaylandquickwlroutputmanagementv1.h
        extensions/aurorawaylandquickxdgoutputv1.cpp extensions/aurorawaylandquickxdgoutputv1.h
        extensions/aurorawaylandwlshellintegration.cpp extensions/aurorawaylandwlshellintegration_p.h
        extensions/aurorawaylandxdgshellintegration.cpp extensions/aurorawaylandxdgshellintegration_p.h
        extensions/aurorawaylandwlrlayershellintegration.cpp extensions/aurorawaylandwlrlayershellintegration_p.h
        extensions/aurorawaylandwlrlayersurfaceitem.cpp extensions/aurorawaylandwlrlayersurfaceitem.h extensions/aurorawaylandwlrlayersurfaceitem_p.h
        extensions/aurorawaylandsurfacelayout.cpp extensions/aurorawaylandsurfacelayout.h extensions/aurorawaylandsurfacelayout_p.h
        extensions/aurorawaylandpresentationtime.cpp extensions/aurorawaylandpresentationtime_p.h extensions/aurorawaylandpresentationtime_p_p.h
        global/aurorawaylandquickextension.h
    CLASS_HEADERS
        WaylandQuickExtension:global/aurorawaylandquickextension.h
    PUBLIC_LIBRARIES
        Qt::Qml
        Qt::Quick
    PKGCONFIG_DEPENDENCIES
        Qt${QT_DEFAULT_MAJOR_VERSION}Qml
        Qt${QT_DEFAULT_MAJOR_VERSION}Quick
)

liri_extend_target(AuroraCompositor CONDITION TARGET Qt::OpenGL AND TARGET Qt::Quick
    SOURCES
        compositor_api/aurorawaylandquickhardwarelayer.cpp compositor_api/aurorawaylandquickhardwarelayer_p.h
)

liri_finalize_module(AuroraCompositor)
