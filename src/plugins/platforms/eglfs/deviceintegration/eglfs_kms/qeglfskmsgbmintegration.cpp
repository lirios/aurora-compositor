/****************************************************************************
**
** Copyright (C) 2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2016 Pelagicore AG
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

#include "qeglfskmsgbmintegration.h"
#include "qeglfskmsgbmdevice.h"
#include "qeglfskmsgbmscreen.h"
#include "qeglfskmsgbmcursor.h"
#include "qeglfskmsgbmwindow.h"
#include "private/qeglfscursor_p.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/QtMath>
#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <qpa/qwindowsysteminterface.h>

#include <LiriAuroraUdev/Udev>
#include <LiriAuroraUdev/UdevEnumerate>
#include <LiriAuroraLogind/Logind>

#include <gbm.h>

using namespace Aurora::PlatformSupport;

QT_BEGIN_NAMESPACE

QEglFSKmsGbmIntegration::QEglFSKmsGbmIntegration()
{
    qCDebug(qLcEglfsKmsDebug, "New DRM/KMS via GBM integration created");

    m_udev = new Aurora::PlatformSupport::Udev();
}

QEglFSKmsGbmIntegration::~QEglFSKmsGbmIntegration()
{
    delete m_udev;
}

#ifndef EGL_EXT_platform_base
typedef EGLDisplay (EGLAPIENTRYP PFNEGLGETPLATFORMDISPLAYEXTPROC) (EGLenum platform, void *native_display, const EGLint *attrib_list);
#endif

#ifndef EGL_PLATFORM_GBM_KHR
#define EGL_PLATFORM_GBM_KHR 0x31D7
#endif

EGLDisplay QEglFSKmsGbmIntegration::createDisplay(EGLNativeDisplayType nativeDisplay)
{
    qCDebug(qLcEglfsKmsDebug, "Querying EGLDisplay");
    EGLDisplay display;

    PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay = nullptr;
    const char *extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (extensions && (strstr(extensions, "EGL_KHR_platform_gbm") || strstr(extensions, "EGL_MESA_platform_gbm"))) {
        getPlatformDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
            eglGetProcAddress("eglGetPlatformDisplayEXT"));
    }

    if (getPlatformDisplay) {
        display = getPlatformDisplay(EGL_PLATFORM_GBM_KHR, nativeDisplay, nullptr);
    } else {
        qCDebug(qLcEglfsKmsDebug, "No eglGetPlatformDisplay for GBM, falling back to eglGetDisplay");
        display = eglGetDisplay(nativeDisplay);
    }

    return display;
}

EGLNativeWindowType QEglFSKmsGbmIntegration::createNativeWindow(QPlatformWindow *platformWindow,
                                                                const QSize &size,
                                                                const QSurfaceFormat &format)
{
    Q_UNUSED(format);
    Q_ASSERT(device());

    QEglFSKmsGbmScreen *screen = static_cast<QEglFSKmsGbmScreen *>(platformWindow->screen());

    gbm_surface *surface =
            gbm_surface_create(static_cast<QEglFSKmsGbmDevice *>(device())->gbmDevice(),
                               size.width(), size.height(), screen->output().drm_format,
                               GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);

    return reinterpret_cast<EGLNativeWindowType>(surface);
}

EGLNativeWindowType QEglFSKmsGbmIntegration::createNativeOffscreenWindow(const QSurfaceFormat &format)
{
    Q_UNUSED(format);
    Q_ASSERT(device());

    gbm_surface *surface = gbm_surface_create(static_cast<QEglFSKmsGbmDevice *>(device())->gbmDevice(),
                                              1, 1,
                                              GBM_FORMAT_XRGB8888,
                                              GBM_BO_USE_RENDERING);

    return reinterpret_cast<EGLNativeWindowType>(surface);
}

void QEglFSKmsGbmIntegration::destroyNativeWindow(EGLNativeWindowType window)
{
    gbm_surface *surface = reinterpret_cast<gbm_surface *>(window);
    gbm_surface_destroy(surface);
}

QPlatformCursor *QEglFSKmsGbmIntegration::createCursor(QPlatformScreen *screen) const
{
#if QT_CONFIG(opengl)
    if (!screenConfig()->hwCursor()) {
        qCDebug(qLcEglfsKmsDebug, "Using plain OpenGL mouse cursor");
        return new QEglFSCursor(screen);
    }
#else
    Q_UNUSED(screen);
#endif
    return nullptr;
}

void QEglFSKmsGbmIntegration::presentBuffer(QPlatformSurface *surface)
{
    if (!Aurora::PlatformSupport::Logind::instance()->isSessionActive())
        return;

    QWindow *window = static_cast<QWindow *>(surface->surface());
    QEglFSKmsGbmScreen *screen = static_cast<QEglFSKmsGbmScreen *>(window->screen()->handle());
    screen->flip();
}

QEglFSWindow *QEglFSKmsGbmIntegration::createWindow(QWindow *window) const
{
    return new QEglFSKmsGbmWindow(window, this);
}

QFunctionPointer QEglFSKmsGbmIntegration::platformFunction(const QByteArray &function) const
{
    auto returnValue = QEglFSKmsIntegration::platformFunction(function);
    if (returnValue)
        return returnValue;

    if (function == Aurora::PlatformSupport::EglFSFunctions::testScreenChangesIdentifier())
        return QFunctionPointer(testScreenChangesStatic);
    else if (function == Aurora::PlatformSupport::EglFSFunctions::applyScreenChangesIdentifier())
        return QFunctionPointer(applyScreenChangesStatic);

    return nullptr;
}

KmsDevice *QEglFSKmsGbmIntegration::createDevice()
{
    QString path = screenConfig()->devicePath();
    if (!path.isEmpty()) {
        qCDebug(qLcEglfsKmsDebug) << "GBM: Using DRM device" << path << "specified in config file";
    } else {
        Aurora::PlatformSupport::UdevEnumerate enumerate(Aurora::PlatformSupport::UdevDevice::PrimaryVideoDevice | Aurora::PlatformSupport::UdevDevice::GenericVideoDevice, m_udev);
        QList<Aurora::PlatformSupport::UdevDevice *> devices = enumerate.scan();
        qCDebug(qLcEglfsKmsDebug) << "Found the following video devices:";
        for (auto device : qAsConst(devices))
            qCDebug(qLcEglfsKmsDebug) << '\t' << device->deviceNode().toLocal8Bit().constData();

        if (Q_UNLIKELY(devices.isEmpty()))
            qFatal("Could not find DRM device!");

        path = devices.first()->deviceNode();
        qCDebug(qLcEglfsKmsDebug) << "Using" << path;
    }

    return new QEglFSKmsGbmDevice(screenConfig(), path);
}

bool QEglFSKmsGbmIntegration::testScreenChangesStatic(const QVector<Aurora::PlatformSupport::ScreenChange> &changes)
{
    Q_UNUSED(changes)
    return true;
}

bool QEglFSKmsGbmIntegration::applyScreenChangesStatic(const QVector<Aurora::PlatformSupport::ScreenChange> &changes)
{
    for (auto &change : qAsConst(changes)) {
        if (!change.screen || !change.enabled)
            continue;

        auto *gbmScreen = static_cast<QEglFSKmsGbmScreen *>(change.screen->handle());
        if (!gbmScreen)
            continue;

        qCDebug(qLcEglfsKmsDebug) << "Apply screen changes to" << gbmScreen->name();

        gbmScreen->setModeChangeRequested(true);

        bool modeChanged = gbmScreen->setMode(change.resolution, qCeil(change.refreshRate / 1000.0f));
        if (!modeChanged) {
            gbmScreen->setModeChangeRequested(false);
            continue;
        }

        gbmScreen->setVirtualPosition(change.position);
        gbmScreen->setScaleFactor(change.scale);

        const auto windows = QGuiApplication::topLevelWindows();
        for (auto *window : windows) {
            if (window->screen() == change.screen) {
                auto *gbmWindow = static_cast<QEglFSKmsGbmWindow *>(window->handle());
                gbmWindow->resizeSurface(gbmScreen->rawGeometry().size());
                gbmWindow->setGeometry(QRect());
            }
        }

        QWindowSystemInterface::handleScreenGeometryChange(change.screen, gbmScreen->geometry(), gbmScreen->availableGeometry());

        gbmScreen->setModeChangeRequested(false);
    }

    return true;
}

QT_END_NAMESPACE
