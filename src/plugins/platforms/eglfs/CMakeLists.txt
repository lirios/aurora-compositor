include(ECMQtDeclareLoggingCategory)
ecm_qt_declare_logging_category(
    AuroraEglFS_SOURCES
    HEADER "eglfscategories.h"
    IDENTIFIER "gLcEglFS"
    CATEGORY_NAME "aurora.eglfs"
    DEFAULT_SEVERITY "Info"
    DESCRIPTION "Aurora EGLFS Qt platform plugin"
)

qt6_add_plugin(AuroraEglFSPlatformPlugin
    SHARED
    CLASS_NAME EglFSIntegrationPlugin
    MANUAL_FINALIZATION
    eglfsconfigchooser.cpp eglfsconfigchooser.h
    eglfscontext.cpp eglfscontext.h
    eglfscursor.cpp eglfscursor.h
    eglfsinfo.cpp eglfsinfo.h
    eglfsinputmanager.cpp eglfsinputmanager.h
    eglfsintegration.cpp eglfsintegration.h
    eglfsmain.cpp
    eglfsscreen.cpp eglfsscreen.h
    eglfswindow.cpp eglfswindow.h
    aurora-eglfs.json
    ${AuroraEglFS_SOURCES}
)

set_target_properties(AuroraEglFSPlatformPlugin
    PROPERTIES OUTPUT_NAME aurora-eglfs
)

target_compile_definitions(AuroraEglFSPlatformPlugin PRIVATE QT_EGL_NO_X11)

target_link_libraries(AuroraEglFSPlatformPlugin
    PRIVATE
        Qt6::Core
        Qt6::CorePrivate
        Qt6::DBus
        Qt6::Gui
        Qt6::GuiPrivate
        Fontconfig::Fontconfig
        EGL::EGL
        Liri::AuroraPlatform
        Liri::AuroraPlatformPrivate
)

qt6_finalize_target(AuroraEglFSPlatformPlugin)

install(
    TARGETS AuroraEglFSPlatformPlugin
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/platforms
)