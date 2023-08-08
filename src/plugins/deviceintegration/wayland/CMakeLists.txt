include(ECMQtDeclareLoggingCategory)
ecm_qt_declare_logging_category(
    AuroraDeviceIntegrationWayland_SOURCES
    HEADER "waylandloggingcategories.h"
    IDENTIFIER "Aurora::Platform::gLcWayland"
    CATEGORY_NAME "aurora.platform.wayland"
    DEFAULT_SEVERITY "Info"
    DESCRIPTION "Aurora device integration for Wayland"
)

qt6_add_plugin(AuroraDeviceIntegrationWayland
    SHARED
    CLASS_NAME WaylandIntegrationPlugin
    MANUAL_FINALIZATION
    eglintegration.cpp eglintegration.h
    waylandbackend.cpp waylandbackend.h
    waylandcursor.cpp waylandcursor.h
    waylandinputmanager.cpp waylandinputmanager.h
    waylandintegrationplugin.cpp waylandintegrationplugin.h
    waylandkeyboard.cpp waylandkeyboard.h
    waylandintegration.cpp waylandintegration.h
    waylandoutput.cpp waylandoutput.h
    waylandpointer.cpp waylandpointer.h
    waylandtouch.cpp waylandtouch.h
    waylandwindow.cpp waylandwindow.h
    wayland.json
    ${AuroraDeviceIntegrationWayland_SOURCES}        
)

set_target_properties(AuroraDeviceIntegrationWayland
    PROPERTIES OUTPUT_NAME wayland
)

target_link_libraries(AuroraDeviceIntegrationWayland
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Liri::AuroraCore
        Liri::AuroraPlatform
        EGL::EGL
        Wayland::Egl
    PRIVATE
        Liri::AuroraPlatformPrivate
        Plasma::KWaylandClient
)

qt6_finalize_target(AuroraDeviceIntegrationWayland)

install(
    TARGETS AuroraDeviceIntegrationWayland
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/aurora/deviceintegration
)
