// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QEventLoop>
#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QScreen>
#ifndef QT_NO_OPENGL
#  include <QOpenGLContext>
#  include <QOffscreenSurface>
#endif

#include <QtGui/private/qguiapplication_p.h>

#include <qpa/qplatformwindow.h>
#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatforminputcontextfactory_p.h>

#ifndef QT_NO_OPENGL
#  include <QtGui/private/qeglpbuffer_p.h>
#endif
#include <QtGui/private/qgenericunixfontdatabase_p.h>
#include <QtGui/private/qgenericunixservices_p.h>
#include <QtGui/private/qgenericunixthemes_p.h>
#include <QtGui/private/qgenericunixeventdispatcher_p.h>

#include <LiriAuroraPlatform/private/eglfsdeviceintegration_p.h>

#include "eglfscategories.h"
#include "eglfsinfo.h"
#include "eglfsinputmanager.h"
#include "eglfsintegration.h"
#include "eglfsscreen.h"
#include "eglfswindow.h"

EglFSIntegration::EglFSIntegration(const EglFSIntegrationOptions &options)
    : QPlatformIntegration()
    , QPlatformNativeInterface()
    , m_options(options)
    , m_loop(new QEventLoop(this))
    , m_fontDb(new QGenericUnixFontDatabase())
    , m_services(new QGenericUnixServices())
{
    auto *integration = Aurora::Platform::auroraDeviceIntegration();
    connect(integration, &Aurora::Platform::DeviceIntegration::statusChanged, this,
            &EglFSIntegration::handleIntegrationStatusChanged, Qt::QueuedConnection);
    connect(integration, &Aurora::Platform::DeviceIntegration::outputAdded, this,
            &EglFSIntegration::handleOutputAdded);
    connect(integration, &Aurora::Platform::DeviceIntegration::outputRemoved, this,
            &EglFSIntegration::handleOutputRemoved);
}

EglFSIntegration::~EglFSIntegration()
{
}

void EglFSIntegration::initialize()
{
    Aurora::Platform::auroraDeviceIntegration()->initialize();

    // Wait until the integration is ready
    m_loop->exec();
    if (!m_ready) {
        destroy();
        qCFatal(gLcEglFS, "Aborting...");
    }

    // Create EGL display
    // auto nativeDisplay = Aurora::Platform::auroraDeviceIntegration()->platformDisplay();
    m_display = Aurora::Platform::auroraDeviceIntegration()->eglDisplay();
    if (Q_UNLIKELY(m_display == EGL_NO_DISPLAY))
        qCFatal(gLcEglFS, "Failed to open EGL display, cannot continue");

    // Initialize EGL
    EGLint major, minor;
    if (Q_UNLIKELY(!eglInitialize(m_display, &major, &minor)))
        qCFatal(gLcEglFS, "Could not initialize EGL display, cannot continue");

    m_inputContext = QPlatformInputContextFactory::create();

    if (!m_options.disableInputHandlers)
        m_inputManager.reset(new EglFSInputManager());
}

void EglFSIntegration::destroy()
{
    qCDebug(gLcEglFS) << "Device integration is about to be destroyed...";

    Aurora::Platform::auroraDeviceIntegration()->destroy();

    m_display = EGL_NO_DISPLAY;
}

EGLDisplay EglFSIntegration::display() const
{
    return m_display;
}

QAbstractEventDispatcher *EglFSIntegration::createEventDispatcher() const
{
    return createUnixEventDispatcher();
}

QPlatformFontDatabase *EglFSIntegration::fontDatabase() const
{
    return m_fontDb.data();
}

QPlatformServices *EglFSIntegration::services() const
{
    return m_services.data();
}

QPlatformInputContext *EglFSIntegration::inputContext() const
{
    return m_inputContext;
}

QPlatformTheme *EglFSIntegration::createPlatformTheme(const QString &name) const
{
    return QGenericUnixTheme::createUnixTheme(name);
}

QPlatformWindow *EglFSIntegration::createPlatformWindow(QWindow *window) const
{
    QWindowSystemInterface::flushWindowSystemEvents(QEventLoop::ExcludeUserInputEvents);

    // Only toplevel windows with OpenGL surfaces are allowed since this platform plugin is
    // tailored for Aurora Wayland compositors that all use QtQuick
    if (window->transientParent()) {
        qCWarning(gLcEglFS) << "Window" << window
                            << "has a transient parent: only toplevel windows are allowed";
        return nullptr;
    }
    if (window->surfaceType() == QWindow::SurfaceType::RasterSurface
        || window->surfaceType() == QWindow::SurfaceType::RasterGLSurface) {
        qCWarning(gLcEglFS) << "Window" << window << "has a raster surface which is not allowed";
        return nullptr;
    }

    // Only a window for screen is allowed
    const auto topLevelWindows = QGuiApplication::topLevelWindows();
    for (auto *topLevelWindow : topLevelWindows) {
        if (topLevelWindow != window && topLevelWindow->screen() == window->screen()) {
            qCWarning(gLcEglFS) << "Window" << window
                                << "cannot be created: there is already a window on the same screen"
                                << window->screen()->name();
            return nullptr;
        }
    }

    // Create the platform window
    auto *platformWindow = new EglFSWindow(const_cast<EglFSIntegration *>(this), window);
    if (!platformWindow->create()) {
        delete platformWindow;
        return nullptr;
    }

    // Show without activating
    const auto showWithoutActivating = window->property("_q_showWithoutActivating");
    if (showWithoutActivating.isValid() && showWithoutActivating.toBool())
        return platformWindow;

    // Activate only the window for the primary screen to make input work
    if (window->type() != Qt::ToolTip && window->screen() == QGuiApplication::primaryScreen())
        platformWindow->requestActivateWindow();

    return platformWindow;
}

QPlatformBackingStore *EglFSIntegration::createPlatformBackingStore(QWindow *window) const
{
    Q_UNUSED(window);
    return nullptr;
}

#ifndef QT_NO_OPENGL
QPlatformOpenGLContext *EglFSIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    QPlatformOpenGLContext *share = context->shareHandle();

    QSurfaceFormat adjustedFormat =
            Aurora::Platform::auroraDeviceIntegration()->surfaceFormatFor(context->format());

    EGLConfig config =
            Aurora::Platform::auroraDeviceIntegration()->chooseConfig(m_display, adjustedFormat);
    EglFSContext *ctx = new EglFSContext(adjustedFormat, share, m_display, &config);

    // Print some information
    eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx->eglContext());
    logGLInfo();
    logEGLInfo(m_display);
    if (config)
        logEGLConfigInfo(m_display, config);
    eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    return ctx;
}

QOpenGLContext *EglFSIntegration::createOpenGLContext(EGLContext context, EGLDisplay contextDisplay,
                                                      QOpenGLContext *shareContext) const
{
    return QEGLPlatformContext::createFrom<EglFSContext>(context, contextDisplay, display(),
                                                         shareContext);
}

QPlatformOffscreenSurface *
EglFSIntegration::createPlatformOffscreenSurface(QOffscreenSurface *surface) const
{
    Q_UNUSED(surface);
    return nullptr;
}
#endif

#if QT_CONFIG(vulkan)
QPlatformVulkanInstance *
EglFSIntegration::createPlatformVulkanInstance(QVulkanInstance *instance) const
{
    Q_UNUSED(instance);
    return nullptr;
}
#endif

bool EglFSIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps:
        return true;
#ifndef QT_NO_OPENGL
    case OpenGL:
        return true;
    case ThreadedOpenGL:
        return true;
    case RasterGLSurface:
        return false;
#else
    case OpenGL:
        return false;
    case ThreadedOpenGL:
        return false;
    case RasterGLSurface:
        return false;
#endif
    case WindowMasks:
        return false;
    case MultipleWindows:
        return true;
    case ForeignWindows:
        return false;
    case WindowManagement:
        return false;
    case WindowActivation:
        return true;
    case MaximizeUsingFullscreenGeometry:
        return true;
    case NonFullScreenWindows:
        return false;
    case OpenGLOnRasterSurface:
        return false;
    case ApplicationState:
        return false;
    default:
        return QPlatformIntegration::hasCapability(cap);
    }
}

QPlatformNativeInterface *EglFSIntegration::nativeInterface() const
{
    return const_cast<EglFSIntegration *>(this);
}

void *EglFSIntegration::nativeResourceForIntegration(const QByteArray &resource)
{
    Q_UNUSED(resource);
    return nullptr;
}

void *EglFSIntegration::nativeResourceForScreen(const QByteArray &resource, QScreen *screen)
{
    Q_UNUSED(resource);
    Q_UNUSED(screen);
    return nullptr;
}

void *EglFSIntegration::nativeResourceForWindow(const QByteArray &resource, QWindow *window)
{
    Q_UNUSED(resource);
    Q_UNUSED(window);
    return nullptr;
}

#ifndef QT_NO_OPENGL
void *EglFSIntegration::nativeResourceForContext(const QByteArray &resource,
                                                 QOpenGLContext *context)
{
    Q_UNUSED(resource);
    Q_UNUSED(context);
    return nullptr;
}
#endif

QPlatformNativeInterface::NativeResourceForContextFunction
EglFSIntegration::nativeResourceFunctionForContext(const QByteArray &resource)
{
    Q_UNUSED(resource);
    return NativeResourceForContextFunction();
}

QFunctionPointer EglFSIntegration::platformFunction(const QByteArray &function) const
{
    Q_UNUSED(function);
    return QFunctionPointer();
}

QVariant EglFSIntegration::styleHint(QPlatformIntegration::StyleHint hint) const
{
    return QPlatformIntegration::styleHint(hint);
}

void EglFSIntegration::handleIntegrationStatusChanged(
        Aurora::Platform::DeviceIntegration::Status status)
{
    switch (status) {
    case Aurora::Platform::DeviceIntegration::Status::Ready:
        qCInfo(gLcEglFS) << "Device integration initialized successfully";
        m_ready = true;
        m_loop->quit();
        break;
    case Aurora::Platform::DeviceIntegration::Status::Failed:
        qCWarning(gLcEglFS) << "Device integration initialization failed";
        m_ready = false;
        m_loop->quit();
        break;
    default:
        break;
    }
}

void EglFSIntegration::handleOutputAdded(Aurora::Platform::Output *output)
{
    const auto isPrimary = QGuiApplication::screens().length() == 0;
    auto *platformScreen = new EglFSScreen(output);
    output->setScreen(platformScreen->screen());
    QWindowSystemInterface::handleScreenAdded(platformScreen, isPrimary);
    qCDebug(gLcEglFS) << "Creating screen" << platformScreen << "(" << platformScreen->name() << ")"
                      << "with geometry" << platformScreen->geometry() << "and isPrimary"
                      << isPrimary;
}

void EglFSIntegration::handleOutputRemoved(Aurora::Platform::Output *output)
{
    const auto screens = QGuiApplication::screens();
    for (auto *screen : qAsConst(screens)) {
        auto *platformScreen = static_cast<EglFSScreen *>(screen->handle());
        if (platformScreen && platformScreen->auroraOutput() == output) {
            QWindowSystemInterface::handleScreenRemoved(platformScreen);
            break;
        }
    }
}
