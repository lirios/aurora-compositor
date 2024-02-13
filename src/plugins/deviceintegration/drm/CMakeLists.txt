include(ECMQtDeclareLoggingCategory)
ecm_qt_declare_logging_category(
    AuroraDeviceIntegrationDrm_SOURCES
    HEADER "drmloggingcategories.h"
    IDENTIFIER "Aurora::Platform::gLcDrm"
    CATEGORY_NAME "aurora.platform.drm"
    DEFAULT_SEVERITY "Info"
    DESCRIPTION "Aurora device integration for DRM/KMS"
)

qt6_add_plugin(AuroraDeviceIntegrationDrm
    SHARED
    CLASS_NAME DrmIntegrationPlugin
    MANUAL_FINALIZATION
    drmbackend.cpp drmbackend.h
    drmblob.cpp drmblob.h
    drmcrtc.cpp drmcrtc.h
    drmconnector.cpp drmconnector.h
    drmdevice.cpp drmdevice.h
    drmeventreader.cpp drmeventreader.h
    drmintegration.cpp drmintegration.h
    drmintegrationplugin.cpp drmintegrationplugin.h
    drmobject.cpp drmobject.h
    drmoutput.cpp drmoutput.h
    drmplane.cpp drmplane.h
    drmpointer.h
    drmproperty.cpp drmproperty.h
    drmwindow.cpp drmwindow.h
    filedescriptor.cpp filedescriptor.h
    udev.cpp udev.h
    ${AuroraDeviceIntegrationDrm_SOURCES}
)

set_target_properties(AuroraDeviceIntegrationDrm
    PROPERTIES OUTPUT_NAME drm
)

target_link_libraries(AuroraDeviceIntegrationDrm
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Liri::AuroraCore
        Liri::AuroraPlatform
        EGL::EGL
    PRIVATE
        PkgConfig::Libdrm
        PkgConfig::Gbm
        PkgConfig::Libxcvt
        PkgConfig::Libudev
        Liri::AuroraPlatformPrivate
)

qt6_finalize_target(AuroraDeviceIntegrationDrm)

install(
    TARGETS AuroraDeviceIntegrationDrm
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/aurora/deviceintegration
)
