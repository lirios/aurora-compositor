// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QImage>

#include "drmbackend.h"
#include "drmdevice.h"
#include "drmintegration.h"
#include "drmloggingcategories.h"
#include "drmoutput.h"
#include "drmwindow.h"

#include <gbm.h>

namespace Aurora {

namespace Platform {

DrmIntegration::DrmIntegration(QObject *parent)
    : DeviceIntegration(parent)
{
}

DrmIntegration::~DrmIntegration()
{
}

void DrmIntegration::initialize()
{
    DrmBackend::instance()->initialize();
}

void DrmIntegration::destroy()
{
    qCDebug(gLcDrm, "DRM device integration is about to be destroyed...");

    DrmBackend::instance()->destroy();

    qCInfo(gLcDrm, "DRM device integration destroyed successfully");
}

EGLNativeDisplayType DrmIntegration::platformDisplay() const
{
    return DrmBackend::instance()->platformDisplay();
}

EGLDisplay DrmIntegration::eglDisplay() const
{
    return DrmBackend::instance()->eglDisplay();
}

EGLNativeWindowType DrmIntegration::createNativeWindow(Window *window, const QSize &size,
                                                       const QSurfaceFormat &format)
{
    Q_UNUSED(format)

    auto *drmOutput = static_cast<DrmOutput *>(window->output());
    auto *gbmDevice = DrmBackend::instance()->primaryDevice()->gbmDevice();
    auto *gbmSurface =
            gbm_surface_create(gbmDevice, size.width(), size.height(), drmOutput->format(),
                               GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);

    return reinterpret_cast<EGLNativeWindowType>(gbmSurface);
}

void DrmIntegration::destroyNativeWindow(EGLNativeWindowType nativeWindow)
{
    auto *surface = reinterpret_cast<gbm_surface *>(nativeWindow);
    gbm_surface_destroy(surface);
}

QSurfaceFormat DrmIntegration::surfaceFormatFor(const QSurfaceFormat &inputFormat) const
{
    QSurfaceFormat format(inputFormat);
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setRedBufferSize(8);
    format.setGreenBufferSize(8);
    format.setBlueBufferSize(8);
    return format;
}

void DrmIntegration::waitForVSync(Window *window) const
{
    Q_UNUSED(window)
}

void DrmIntegration::presentBuffer(Window *window)
{
    Q_UNUSED(window)
}

Window *DrmIntegration::createWindow(Output *output, QWindow *qtWindow)
{
    // return new DrmWindow(output, qtWindow, this);
    return nullptr;
}

Window *DrmIntegration::getWindow(QWindow *qtWindow) const
{
    return nullptr;
}

InputManager *DrmIntegration::createInputManager(QObject *parent)
{
    Q_UNUSED(parent)
    return nullptr;
}

Outputs DrmIntegration::outputs() const
{
    return Outputs();
}

} // namespace Platform

} // namespace Aurora
