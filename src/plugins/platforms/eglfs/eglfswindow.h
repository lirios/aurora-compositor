// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <qpa/qplatformwindow.h>

#include <LiriAuroraPlatform/DeviceIntegration>

#include "eglfsscreen.h"

class EglFSIntegration;

class EglFSWindow : public QPlatformWindow
{
public:
    EglFSWindow(EglFSIntegration *integration, QWindow *window);
    ~EglFSWindow();

    Aurora::Platform::Window *auroraWindow() const;

    bool create();

    WId winId() const override;

    EglFSScreen *screen() const override;

    EGLNativeWindowType eglWindow() const;
    EGLSurface surface() const;

    QRect geometry() const override;
    void setGeometry(const QRect &rect) override;

    void setVisible(bool visible) override;
    void requestActivateWindow() override;
    void raise() override;
    void lower() override;

    void propagateSizeHints() override
    {
    }

    bool setKeyboardGrabEnabled(bool) override
    {
        return false;
    }
    bool setMouseGrabEnabled(bool) override
    {
        return false;
    }

    void setOpacity(qreal level) override;
    void setMask(const QRegion &region) override;

    QSurfaceFormat format() const override;

#if QT_CONFIG(vulkan)
    virtual void *vulkanSurfacePtr()
    {
        return nullptr;
    }
#endif

    void invalidateSurface() override;
    virtual void resetSurface();

    bool isRaster() const;

private:
    QPointer<EglFSIntegration> m_integration;
    WId m_winId = 0;
    bool m_created = false;
    QScopedPointer<Aurora::Platform::Window> m_window;

    EGLSurface m_eglSurface = EGL_NO_SURFACE;
    EGLConfig m_config = nullptr;
    EGLNativeWindowType m_eglWindow = EGL_CAST(EGLNativeWindowType, 0);
    QSurfaceFormat m_format;

    void destroy();
    bool createSurface();
};
