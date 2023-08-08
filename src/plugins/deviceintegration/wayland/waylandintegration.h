// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <LiriAuroraPlatform/DeviceIntegration>

struct wl_display;

namespace Aurora {

namespace Platform {

class EglIntegration;
class WaylandWindow;

class WaylandIntegration : public DeviceIntegration
{
    Q_OBJECT
public:
    explicit WaylandIntegration(QObject *parent = nullptr);
    ~WaylandIntegration();

    void initialize() override;
    void destroy() override;

    EGLNativeDisplayType platformDisplay() const override;
    EGLDisplay eglDisplay() const override;

    wl_display *display() const;

    EGLNativeWindowType createNativeWindow(Window *window, const QSize &size,
                                           const QSurfaceFormat &format) override;
    void destroyNativeWindow(EGLNativeWindowType nativeWindow) override;

    void waitForVSync(Window *window) const override;
    void presentBuffer(Window *window) override;

    Window *createWindow(Output *output, QWindow *qtWindow) override;
    Window *getWindow(QWindow *qtWindow) const override;

    InputManager *createInputManager(QObject *parent = nullptr) override;

    Outputs outputs() const override;

private:
    QScopedPointer<EglIntegration> m_eglIntegration;
};

} // namespace Platform

} // namespace Aurora
