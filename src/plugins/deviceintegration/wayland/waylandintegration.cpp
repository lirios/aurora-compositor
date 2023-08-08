// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QImage>

#include "eglintegration.h"
#include "waylandbackend.h"
#include "waylandinputmanager.h"
#include "waylandintegration.h"
#include "waylandloggingcategories.h"
#include "waylandoutput.h"
#include "waylandwindow.h"

#include <wayland-egl.h>

namespace Aurora {

namespace Platform {

WaylandIntegration::WaylandIntegration(QObject *parent)
    : DeviceIntegration(parent)
    , m_eglIntegration(new EglIntegration())
{
}

WaylandIntegration::~WaylandIntegration()
{
}

void WaylandIntegration::initialize()
{
    connect(WaylandBackend::instance(), &WaylandBackend::statusChanged, this,
            [this](DeviceIntegration::Status status) { DeviceIntegration::setStatus(status); });
    connect(WaylandBackend::instance(), &WaylandBackend::outputAdded, this,
            &WaylandIntegration::outputAdded);
    connect(WaylandBackend::instance(), &WaylandBackend::outputRemoved, this,
            &WaylandIntegration::outputRemoved);

    WaylandBackend::instance()->initialize();

    if (m_eglIntegration)
        m_eglIntegration->initialize();
}

void WaylandIntegration::destroy()
{
    qCDebug(gLcWayland, "Wayland device integration is about to be destroyed...");

    WaylandBackend::instance()->destroy();

    if (m_eglIntegration)
        m_eglIntegration->destroy();

    qCInfo(gLcWayland, "Wayland device integration destroyed successfully");
}

EGLNativeDisplayType WaylandIntegration::platformDisplay() const
{
    return reinterpret_cast<EGLNativeDisplayType>(display());
}

EGLDisplay WaylandIntegration::eglDisplay() const
{
    if (m_eglIntegration && m_eglIntegration->isInitialized())
        return m_eglIntegration->display();
    return EGL_NO_DISPLAY;
}

wl_display *WaylandIntegration::display() const
{
    return WaylandBackend::instance()->display();
}

EGLNativeWindowType WaylandIntegration::createNativeWindow(Window *window, const QSize &size,
                                                           const QSurfaceFormat &format)
{
    Q_UNUSED(format)

    auto *wlWindow = static_cast<WaylandWindow *>(window);
    if (!wlWindow) {
        qCWarning(gLcWayland) << "Window" << window << "cannot be cast to WaylandWindow";
        return 0;
    }

    auto *surface = wlWindow->surface()->operator wl_surface *();
    if (surface) {
        qCDebug(gLcWayland, "Creating native window with size %dx%d...", size.width(),
                size.height());
        return reinterpret_cast<EGLNativeWindowType>(
                wl_egl_window_create(surface, size.width(), size.height()));
    } else {
        qCWarning(gLcWayland) << "Unable to get Wayland surface from window" << window;
        return 0;
    }

    return 0;
}

void WaylandIntegration::destroyNativeWindow(EGLNativeWindowType nativeWindow)
{
    if (nativeWindow) {
        auto *eglWindow = reinterpret_cast<wl_egl_window *>(nativeWindow);
        wl_egl_window_destroy(eglWindow);
    }
}

void WaylandIntegration::waitForVSync(Window *window) const
{
    Q_UNUSED(window);
}

void WaylandIntegration::presentBuffer(Window *window)
{
    auto *waylandWindow = static_cast<WaylandWindow *>(window);
    if (waylandWindow)
        waylandWindow->present();
}

Window *WaylandIntegration::createWindow(Output *output, QWindow *qtWindow)
{
    return new WaylandWindow(output, qtWindow);
}

Window *WaylandIntegration::getWindow(QWindow *qtWindow) const
{
    const auto windows = WaylandBackend::instance()->windows();
    for (auto *window : windows) {
        if (window->qtWindow() == qtWindow)
            return static_cast<Window *>(window);
    }

    return nullptr;
}

InputManager *WaylandIntegration::createInputManager(QObject *parent)
{
    return new WaylandInputManager(parent);
}

Outputs WaylandIntegration::outputs() const
{
    return WaylandBackend::instance()->outputs();
}

} // namespace Platform

} // namespace Aurora
