// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QSurfaceFormat>

#include <LiriAuroraPlatform/Output>

#include <EGL/egl.h>

#include <memory>

class QPlatformSurface;

namespace Aurora {

namespace Platform {

class DeviceIntegrationPrivate;
class InputManager;
class Window;

class LIRIAURORAPLATFORM_EXPORT DeviceIntegration : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_DECLARE_PRIVATE(DeviceIntegration)
public:
    enum class Status {
        NotReady,
        Ready,
        Failed
    };
    Q_ENUM(Status)

    ~DeviceIntegration();

    Status status() const;

    virtual void initialize() = 0;
    virtual void destroy() = 0;

    virtual bool supportsPBuffers();
    virtual bool supportsSurfacelessContexts();

    virtual EGLNativeDisplayType platformDisplay() const;
    virtual EGLDisplay eglDisplay() const;

    virtual EGLNativeWindowType createNativeWindow(Window *window, const QSize &size,
                                                   const QSurfaceFormat &format);
    virtual void destroyNativeWindow(EGLNativeWindowType nativeWindow);

    virtual QSurfaceFormat surfaceFormatFor(const QSurfaceFormat &inputFormat) const;
    virtual EGLint surfaceType() const;

    virtual EGLConfig chooseConfig(EGLDisplay display, const QSurfaceFormat &format);

    virtual Window *createWindow(Output *output, QWindow *qtWindow) = 0;
    virtual Window *getWindow(QWindow *qtWindow) const;

    virtual void waitForVSync(Window *window) const = 0;
    virtual void presentBuffer(Window *window) = 0;

    virtual InputManager *createInputManager(QObject *parent = nullptr);

    virtual Outputs outputs() const = 0;

Q_SIGNALS:
    void statusChanged(Status status);
    void outputAdded(Output *output);
    void outputRemoved(Output *output);

protected:
    QScopedPointer<DeviceIntegrationPrivate> const d_ptr;

    explicit DeviceIntegration(QObject *parent = nullptr);

    void setStatus(Status status);
};

} // namespace Platform

} // namespace Aurora
