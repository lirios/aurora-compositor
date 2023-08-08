// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPointer>

#ifndef EGL_NO_X11
#  define EGL_NO_X11
#endif
#ifndef MESA_EGL_NO_X11_HEADERS
#  define MESA_EGL_NO_X11_HEADERS
#endif

#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace Aurora {

namespace Platform {

class EglIntegration
{
public:
    EglIntegration();
    ~EglIntegration();

    bool isInitialized() const;

    EGLDisplay display() const;

    bool initialize();
    void destroy();

    static bool hasEglExtension(const char *name, EGLDisplay display = EGL_NO_DISPLAY);

private:
    bool m_initialized = false;
    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
};

} // namespace Platform

} // namespace Aurora
