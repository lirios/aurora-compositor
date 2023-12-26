/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/qtextstream.h>
#include <QtGui/private/qguiapplication_p.h>

#include <qpa/qplatformwindow.h>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QScreen>
#ifndef QT_NO_OPENGL
# include <QtGui/QOpenGLContext>
# include <QtGui/QOffscreenSurface>
#endif
#include <QtGui/QWindow>
#include <QtCore/QLoggingCategory>
#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatforminputcontextfactory_p.h>

#include "qeglfsintegration_p.h"
#include "qeglfswindow_p.h"
#include "qeglfshooks_p.h"
#ifndef QT_NO_OPENGL
# include "qeglfscontext_p.h"
# include "qeglfscursor_p.h"
#endif
#include "qeglfsoffscreenwindow_p.h"
#include "qeglfslogindhandler_p.h"
#include "vthandler.h"
#include "libinputmanager_p.h"

#include <QtEglSupport/private/qeglconvenience_p.h>
#ifndef QT_NO_OPENGL
# include <QtEglSupport/private/qeglplatformcontext_p.h>
# include <QtEglSupport/private/qeglpbuffer_p.h>
#endif

#include <QtFontDatabaseSupport/private/qgenericunixfontdatabase_p.h>
#include <QtServiceSupport/private/qgenericunixservices_p.h>
#include <QtThemeSupport/private/qgenericunixthemes_p.h>
#include <QtEventDispatcherSupport/private/qgenericunixeventdispatcher_p.h>
#include <QtFbSupport/private/qfbvthandler_p.h>
#ifndef QT_NO_OPENGL
# include <QtPlatformCompositorSupport/private/qopenglcompositorbackingstore_p.h>
#endif

#include <QtPlatformHeaders/QEGLNativeContext>

#include <QtPlatformHeaders/qeglfsfunctions.h>

using namespace Aurora::PlatformSupport;

static void initResources()
{
#ifndef QT_NO_CURSOR
    Q_INIT_RESOURCE(cursor);
#endif
}

QT_BEGIN_NAMESPACE

QEglFSIntegration::QEglFSIntegration()
    : m_display(EGL_NO_DISPLAY),
      m_inputContext(nullptr),
      m_fontDb(new QGenericUnixFontDatabase),
      m_services(new QGenericUnixServices),
      m_logindHandler(nullptr),
      m_disableInputHandlers(false)
{
    m_disableInputHandlers = qEnvironmentVariableIntValue("QT_QPA_EGLFS_DISABLE_INPUT");

    initResources();
}

void QEglFSIntegration::initialize()
{
    m_logindHandler = new QEglFSLogindHandler();
    connect(m_logindHandler, &QEglFSLogindHandler::initializationRequested, this, [&] {
        qt_egl_device_integration()->platformInit();

        m_display = qt_egl_device_integration()->createDisplay(nativeDisplay());
        if (Q_UNLIKELY(m_display == EGL_NO_DISPLAY))
            qFatal("Could not open egl display");

        EGLint major, minor;
        if (Q_UNLIKELY(!eglInitialize(m_display, &major, &minor)))
            qFatal("Could not initialize egl display");

        m_inputContext = QPlatformInputContextFactory::create();

        m_vtHandler.reset(new VtHandler);

        if (qt_egl_device_integration()->usesDefaultScreen())
            QWindowSystemInterface::handleScreenAdded(new QEglFSScreen(display()));
        else
            qt_egl_device_integration()->screenInit();

        // Input code may rely on the screens, so do it only after the screen init.
        if (!m_disableInputHandlers)
            createInputHandlers();

        // Exit initialization
        m_logindHandler->stop();
    });
    m_logindHandler->initialize();
}

void QEglFSIntegration::destroy()
{
    foreach (QWindow *w, qGuiApp->topLevelWindows())
        w->destroy();

    qt_egl_device_integration()->screenDestroy();

    if (m_display != EGL_NO_DISPLAY)
        eglTerminate(m_display);

    qt_egl_device_integration()->platformDestroy();

    m_logindHandler->deleteLater();
}

QAbstractEventDispatcher *QEglFSIntegration::createEventDispatcher() const
{
    return createUnixEventDispatcher();
}

QPlatformServices *QEglFSIntegration::services() const
{
    return m_services.data();
}

QPlatformFontDatabase *QEglFSIntegration::fontDatabase() const
{
    return m_fontDb.data();
}

QPlatformTheme *QEglFSIntegration::createPlatformTheme(const QString &name) const
{
    return QGenericUnixTheme::createUnixTheme(name);
}

QPlatformBackingStore *QEglFSIntegration::createPlatformBackingStore(QWindow *window) const
{
#ifndef QT_NO_OPENGL
    QOpenGLCompositorBackingStore *bs = new QOpenGLCompositorBackingStore(window);
    if (!window->handle())
        window->create();
    static_cast<QEglFSWindow *>(window->handle())->setBackingStore(bs);
    return bs;
#else
    Q_UNUSED(window);
    return nullptr;
#endif
}

QPlatformWindow *QEglFSIntegration::createPlatformWindow(QWindow *window) const
{
    QWindowSystemInterface::flushWindowSystemEvents(QEventLoop::ExcludeUserInputEvents);
    QEglFSWindow *w = qt_egl_device_integration()->createWindow(window);
    w->create();

    const auto showWithoutActivating = window->property("_q_showWithoutActivating");
    if (showWithoutActivating.isValid() && showWithoutActivating.toBool())
        return w;

    // Activate only the window for the primary screen to make input work
    if (window->type() != Qt::ToolTip && window->screen() == QGuiApplication::primaryScreen())
        w->requestActivateWindow();

    return w;
}

#ifndef QT_NO_OPENGL
QPlatformOpenGLContext *QEglFSIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
    EGLDisplay dpy = context->screen() ? static_cast<QEglFSScreen *>(context->screen()->handle())->display() : display();
    QPlatformOpenGLContext *share = context->shareHandle();
    QVariant nativeHandle = context->nativeHandle();

    QEglFSContext *ctx;
    QSurfaceFormat adjustedFormat = qt_egl_device_integration()->surfaceFormatFor(context->format());
    if (nativeHandle.isNull()) {
        EGLConfig config = QEglFSDeviceIntegration::chooseConfig(dpy, adjustedFormat);
        ctx = new QEglFSContext(adjustedFormat, share, dpy, &config, QVariant());
    } else {
        ctx = new QEglFSContext(adjustedFormat, share, dpy, nullptr, nativeHandle);
    }
    nativeHandle = QVariant::fromValue<QEGLNativeContext>(QEGLNativeContext(ctx->eglContext(), dpy));

    context->setNativeHandle(nativeHandle);
    return ctx;
}

QPlatformOffscreenSurface *QEglFSIntegration::createPlatformOffscreenSurface(QOffscreenSurface *surface) const
{
    EGLDisplay dpy = surface->screen() ? static_cast<QEglFSScreen *>(surface->screen()->handle())->display() : display();
    QSurfaceFormat fmt = qt_egl_device_integration()->surfaceFormatFor(surface->requestedFormat());
    if (qt_egl_device_integration()->supportsPBuffers()) {
        QEGLPlatformContext::Flags flags = {};
        if (!qt_egl_device_integration()->supportsSurfacelessContexts())
            flags |= QEGLPlatformContext::NoSurfaceless;
        return new QEGLPbuffer(dpy, fmt, surface, flags);
    } else {
        return new QEglFSOffscreenWindow(dpy, fmt, surface);
    }
    // Never return null. Multiple QWindows are not supported by this plugin.
}
#endif // QT_NO_OPENGL

#if QT_CONFIG(vulkan)
QPlatformVulkanInstance *QEglFSIntegration::createPlatformVulkanInstance(QVulkanInstance *instance) const
{
    return qt_egl_device_integration()->createPlatformVulkanInstance(instance);
}
#endif

bool QEglFSIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    // We assume that devices will have more and not less capabilities
    if (qt_egl_device_integration()->hasCapability(cap))
        return true;

    switch (cap) {
    case ThreadedPixmaps: return true;
#ifndef QT_NO_OPENGL
    case OpenGL: return true;
    case ThreadedOpenGL: return true;
    case RasterGLSurface: return true;
#else
    case OpenGL: return false;
    case ThreadedOpenGL: return false;
    case RasterGLSurface: return false;
#endif
    case WindowManagement: return false;
    case OpenGLOnRasterSurface: return true;
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPlatformNativeInterface *QEglFSIntegration::nativeInterface() const
{
    return const_cast<QEglFSIntegration *>(this);
}

enum ResourceType {
    EglDisplay,
    EglWindow,
    EglContext,
    EglConfig,
    NativeDisplay,
    XlibDisplay,
    WaylandDisplay,
    EglSurface,
    VkSurface
};

static int resourceType(const QByteArray &key)
{
    static const QByteArray names[] = { // match ResourceType
        QByteArrayLiteral("egldisplay"),
        QByteArrayLiteral("eglwindow"),
        QByteArrayLiteral("eglcontext"),
        QByteArrayLiteral("eglconfig"),
        QByteArrayLiteral("nativedisplay"),
        QByteArrayLiteral("display"),
        QByteArrayLiteral("server_wl_display"),
        QByteArrayLiteral("eglsurface"),
        QByteArrayLiteral("vksurface")
    };
    const QByteArray *end = names + sizeof(names) / sizeof(names[0]);
    const QByteArray *result = std::find(names, end, key);
    if (result == end)
        result = std::find(names, end, key.toLower());
    return int(result - names);
}

void *QEglFSIntegration::nativeResourceForIntegration(const QByteArray &resource)
{
    void *result = nullptr;

    switch (resourceType(resource)) {
    case EglDisplay:
        result = display();
        break;
    case NativeDisplay:
        result = reinterpret_cast<void*>(nativeDisplay());
        break;
    case WaylandDisplay:
        result = qt_egl_device_integration()->wlDisplay();
        break;
    default:
        result = qt_egl_device_integration()->nativeResourceForIntegration(resource);
        break;
    }

    return result;
}

void *QEglFSIntegration::nativeResourceForScreen(const QByteArray &resource, QScreen *screen)
{
    void *result = nullptr;

    switch (resourceType(resource)) {
    case XlibDisplay:
        // Play nice when using the x11 hooks: Be compatible with xcb that allows querying
        // the X Display pointer, which is nothing but our native display.
        result = reinterpret_cast<void*>(nativeDisplay());
        break;
    default:
        result = qt_egl_device_integration()->nativeResourceForScreen(resource, screen);
        break;
    }

    return result;
}

void *QEglFSIntegration::nativeResourceForWindow(const QByteArray &resource, QWindow *window)
{
    void *result = nullptr;

    switch (resourceType(resource)) {
    case EglDisplay:
        if (window && window->handle())
            result = static_cast<QEglFSScreen *>(window->handle()->screen())->display();
        else
            result = display();
        break;
    case EglWindow:
        if (window && window->handle())
            result = reinterpret_cast<void*>(static_cast<QEglFSWindow *>(window->handle())->eglWindow());
        break;
    case EglSurface:
        if (window && window->handle())
            result = reinterpret_cast<void*>(static_cast<QEglFSWindow *>(window->handle())->surface());
        break;
#if QT_CONFIG(vulkan)
    case VkSurface:
        if (window && window->handle() && window->surfaceType() == QSurface::VulkanSurface)
            result = static_cast<QEglFSWindow *>(window->handle())->vulkanSurfacePtr();
        break;
#endif
    default:
        break;
    }

    return result;
}

#ifndef QT_NO_OPENGL
void *QEglFSIntegration::nativeResourceForContext(const QByteArray &resource, QOpenGLContext *context)
{
    void *result = nullptr;

    switch (resourceType(resource)) {
    case EglContext:
        if (context->handle())
            result = static_cast<QEglFSContext *>(context->handle())->eglContext();
        break;
    case EglConfig:
        if (context->handle())
            result = static_cast<QEglFSContext *>(context->handle())->eglConfig();
        break;
    case EglDisplay:
        if (context->handle())
            result = static_cast<QEglFSContext *>(context->handle())->eglDisplay();
        break;
    default:
        break;
    }

    return result;
}

static void *eglContextForContext(QOpenGLContext *context)
{
    Q_ASSERT(context);

    QEglFSContext *handle = static_cast<QEglFSContext *>(context->handle());
    if (!handle)
        return nullptr;

    return handle->eglContext();
}
#endif

QPlatformNativeInterface::NativeResourceForContextFunction QEglFSIntegration::nativeResourceFunctionForContext(const QByteArray &resource)
{
#ifndef QT_NO_OPENGL
    if (resource.compare("get_egl_context", Qt::CaseInsensitive) == 0)
        return NativeResourceForContextFunction(eglContextForContext);
#else
    Q_UNUSED(resource);
#endif
    return nullptr;
}

QFunctionPointer QEglFSIntegration::platformFunction(const QByteArray &function) const
{
    if (function == Aurora::PlatformSupport::EglFSFunctions::setCursorThemeIdentifier())
        return QFunctionPointer(setCursorThemeStatic);
    else if (function == Aurora::PlatformSupport::EglFSFunctions::getPowerStateIdentifier())
        return QFunctionPointer(getPowerStateStatic);
    else if (function == Aurora::PlatformSupport::EglFSFunctions::setPowerStateIdentifier())
        return QFunctionPointer(setPowerStateStatic);
    else if (function == Aurora::PlatformSupport::EglFSFunctions::enableScreenCastIdentifier())
        return QFunctionPointer(enableScreenCastStatic);
    else if (function == Aurora::PlatformSupport::EglFSFunctions::disableScreenCastIdentifier())
        return QFunctionPointer(disableScreenCastStatic);

    return qt_egl_device_integration()->platformFunction(function);
}

void QEglFSIntegration::createInputHandlers()
{
    m_liHandler.reset(new Aurora::PlatformSupport::LibInputManager(this));
}

void QEglFSIntegration::setCursorThemeStatic(const QString &name, int size)
{
    const auto screens = qGuiApp->screens();
    for (auto *screen : screens) {
        auto *platformScreen = static_cast<QEglFSScreen *>(screen->handle());
        if (platformScreen)
            platformScreen->setCursorTheme(name, size);
    }
}

Aurora::PlatformSupport::EglFSFunctions::PowerState QEglFSIntegration::getPowerStateStatic(QScreen *screen)
{
    QPlatformScreen::PowerState powerState = screen->handle()->powerState();
    switch (powerState) {
    case QPlatformScreen::PowerStateOn:
        return Aurora::PlatformSupport::EglFSFunctions::PowerStateOn;
    case QPlatformScreen::PowerStateOff:
        return Aurora::PlatformSupport::EglFSFunctions::PowerStateOff;
    case QPlatformScreen::PowerStateStandby:
        return Aurora::PlatformSupport::EglFSFunctions::PowerStateStandby;
    case QPlatformScreen::PowerStateSuspend:
        return Aurora::PlatformSupport::EglFSFunctions::PowerStateSuspend;
    }
    Q_UNREACHABLE();
}

void QEglFSIntegration::setPowerStateStatic(QScreen *screen, Aurora::PlatformSupport::EglFSFunctions::PowerState powerState)
{
    switch (powerState) {
    case Aurora::PlatformSupport::EglFSFunctions::PowerStateOn:
        screen->handle()->setPowerState(QPlatformScreen::PowerStateOn);
        break;
    case Aurora::PlatformSupport::EglFSFunctions::PowerStateOff:
        screen->handle()->setPowerState(QPlatformScreen::PowerStateOff);
        break;
    case Aurora::PlatformSupport::EglFSFunctions::PowerStateStandby:
        screen->handle()->setPowerState(QPlatformScreen::PowerStateStandby);
        break;
    case Aurora::PlatformSupport::EglFSFunctions::PowerStateSuspend:
        screen->handle()->setPowerState(QPlatformScreen::PowerStateSuspend);
        break;
    }
}

void QEglFSIntegration::enableScreenCastStatic(QScreen *screen)
{
    QEglFSScreen *platformScreen = static_cast<QEglFSScreen *>(screen->handle());
    platformScreen->setRecordingEnabled(true);
}

void QEglFSIntegration::disableScreenCastStatic(QScreen *screen)
{
    QEglFSScreen *platformScreen = static_cast<QEglFSScreen *>(screen->handle());
    platformScreen->setRecordingEnabled(false);
}

EGLNativeDisplayType QEglFSIntegration::nativeDisplay() const
{
    return qt_egl_device_integration()->platformDisplay();
}

QT_END_NAMESPACE
