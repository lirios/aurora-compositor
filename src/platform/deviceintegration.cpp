// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "deviceintegration.h"
#include "deviceintegration_p.h"
#include "eglconfigchooser_p.h"
#include "inputmanager.h"

namespace Aurora {

namespace Platform {

/*
 * DeviceIntegration
 */

DeviceIntegration::DeviceIntegration(QObject *parent)
    : QObject(parent)
    , d_ptr(new DeviceIntegrationPrivate(this))
{
}

DeviceIntegration::~DeviceIntegration()
{
}

DeviceIntegration::Status DeviceIntegration::status() const
{
    Q_D(const DeviceIntegration);
    return d->status;
}

void DeviceIntegration::setStatus(Status status)
{
    Q_D(DeviceIntegration);

    if (d->status == status)
        return;

    d->status = status;
    Q_EMIT statusChanged(status);
}

bool DeviceIntegration::supportsPBuffers()
{
    return true;
}

bool DeviceIntegration::supportsSurfacelessContexts()
{
    return true;
}

EGLNativeDisplayType DeviceIntegration::platformDisplay() const
{
    return EGL_DEFAULT_DISPLAY;
}

EGLDisplay DeviceIntegration::eglDisplay() const
{
    return EGL_NO_DISPLAY;
}

EGLNativeWindowType DeviceIntegration::createNativeWindow(Window *window, const QSize &size,
                                                          const QSurfaceFormat &format)
{
    Q_UNUSED(window)
    Q_UNUSED(size)
    Q_UNUSED(format)
    return 0;
}

void DeviceIntegration::destroyNativeWindow(EGLNativeWindowType nativeWindow)
{
    Q_UNUSED(nativeWindow)
}

QSurfaceFormat DeviceIntegration::surfaceFormatFor(const QSurfaceFormat &inputFormat) const
{
    return inputFormat;
}

EGLint DeviceIntegration::surfaceType() const
{
    return EGL_WINDOW_BIT;
}

EGLConfig DeviceIntegration::chooseConfig(EGLDisplay display, const QSurfaceFormat &format)
{
    QVector<EGLint> configAttribs = eglConfigAttributesFromSurfaceFormat(display, format);

    configAttribs.append(EGL_SURFACE_TYPE);
    configAttribs.append(surfaceType());

    configAttribs.append(EGL_NONE);

    // Get the number of matching configurations for the attributes
    EGLConfig config = nullptr;
    EGLint numConfigs = 0;
    if (!eglChooseConfig(display, configAttribs.constData(), &config, 1, &numConfigs))
        return nullptr;
    return config;
}

Window *DeviceIntegration::getWindow(QWindow *qtWindow) const
{
    Q_UNUSED(qtWindow)
    return nullptr;
}

InputManager *Aurora::Platform::DeviceIntegration::createInputManager(QObject *parent)
{
    Q_UNUSED(parent)
    return nullptr;
}

/*
 * DeviceIntegrationPrivate
 */

DeviceIntegrationPrivate::DeviceIntegrationPrivate(DeviceIntegration *self)
    : q_ptr(self)
{
}

} // namespace Platform

} // namespace Aurora
