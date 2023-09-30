// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <LiriAuroraPlatform/DeviceIntegration>

namespace Aurora {

namespace Platform {

class DrmWindow;

class DrmIntegration : public DeviceIntegration
{
    Q_OBJECT
public:
    explicit DrmIntegration(QObject *parent = nullptr);
    ~DrmIntegration();

    void initialize() override;
    void destroy() override;

    EGLNativeDisplayType platformDisplay() const override;
    EGLDisplay eglDisplay() const override;

    EGLNativeWindowType createNativeWindow(Window *window, const QSize &size,
                                           const QSurfaceFormat &format) override;
    void destroyNativeWindow(EGLNativeWindowType nativeWindow) override;

    QSurfaceFormat surfaceFormatFor(const QSurfaceFormat &inputFormat) const override;

    void waitForVSync(Window *window) const override;
    void presentBuffer(Window *window) override;

    Window *createWindow(Output *output, QWindow *qtWindow) override;
    Window *getWindow(QWindow *qtWindow) const override;

    InputManager *createInputManager(QObject *parent = nullptr) override;

    Outputs outputs() const override;
};

} // namespace Platform

} // namespace Aurora
