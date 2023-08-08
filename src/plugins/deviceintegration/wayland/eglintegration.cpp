// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QByteArray>
#include <QList>

#include "eglintegration.h"
#include "waylandbackend.h"
#include "waylandloggingcategories.h"

namespace Aurora {

namespace Platform {

EglIntegration::EglIntegration()
{
}

EglIntegration::~EglIntegration()
{
    destroy();
}

bool EglIntegration::isInitialized() const
{
    return m_initialized;
}

EGLDisplay EglIntegration::display() const
{
    return m_eglDisplay;
}

typedef const char *(*EGLGETERRORSTRINGPROC)(EGLint error);

bool EglIntegration::initialize()
{
    if (m_initialized)
        return true;

    m_initialized = true;

    if (hasEglExtension("EGL_EXT_platform_base")) {
        if (hasEglExtension("EGL_KHR_platform_wayland")
            || hasEglExtension("EGL_EXT_platform_wayland")
            || hasEglExtension("EGL_MESA_platform_wayland")) {
            static PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplay = nullptr;
            if (!eglGetPlatformDisplay)
                eglGetPlatformDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
                        eglGetProcAddress("eglGetPlatformDisplayEXT"));

            m_eglDisplay = eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_KHR,
                                                 WaylandBackend::instance()->display(), nullptr);
        } else {
            qCWarning(gLcWayland) << "The EGL implementation does not support the Wayland platform";
            return false;
        }
    } else {
        QByteArray eglPlatform = qgetenv("EGL_PLATFORM");
        if (eglPlatform.isEmpty())
            setenv("EGL_PLATFORM", "wayland", true);

        m_eglDisplay = eglGetDisplay(
                reinterpret_cast<EGLNativeDisplayType>(WaylandBackend::instance()->display()));
    }

    return true;
}

void EglIntegration::destroy()
{
    if (m_eglDisplay != EGL_NO_DISPLAY) {
        eglTerminate(m_eglDisplay);
        m_eglDisplay = EGL_NO_DISPLAY;
    }
}

bool EglIntegration::hasEglExtension(const char *name, EGLDisplay display)
{
    QList<QByteArray> extensions =
            QByteArray(reinterpret_cast<const char *>(eglQueryString(display, EGL_EXTENSIONS)))
                    .split(' ');
    return extensions.contains(name);
}

} // namespace Platform

} // namespace Aurora
