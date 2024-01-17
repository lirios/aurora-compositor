// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <qpa/qwindowsysteminterface.h>

#include <QtGui/private/qeglconvenience_p.h>

#include <LiriAuroraPlatform/Window>
#include <LiriAuroraPlatform/private/eglfsdeviceintegration_p.h>

#include "eglfscategories.h"
#include "eglfsconfigchooser.h"
#include "eglfsintegration.h"
#include "eglfswindow.h"

static WId newWinId()
{
    static WId id = 0;

    if (id == std::numeric_limits<WId>::max())
        qCWarning(gLcEglFS, "Out of window IDs");

    return ++id;
}

EglFSWindow::EglFSWindow(EglFSIntegration *integration, QWindow *window)
    : QPlatformWindow(window)
    , m_integration(integration)
    , m_winId(newWinId())
    , m_format(window->requestedFormat())
{
    m_window.reset(Aurora::Platform::auroraDeviceIntegration()->createWindow(
            screen()->auroraOutput(), window));
}

EglFSWindow::~EglFSWindow()
{
    destroy();
}

Aurora::Platform::Window *EglFSWindow::auroraWindow() const
{
    return m_window.data();
}

bool EglFSWindow::create()
{
    if (m_created)
        return true;

    m_created = true;

    // Windows are full screen
    setGeometry(screen()->geometry());

    if (!m_window->create())
        return false;

    if (!createSurface())
        return false;

    return true;
}

WId EglFSWindow::winId() const
{
    return m_winId;
}

EglFSScreen *EglFSWindow::screen() const
{
    return static_cast<EglFSScreen *>(QPlatformWindow::screen());
}

EGLNativeWindowType EglFSWindow::eglWindow() const
{
    return m_eglWindow;
}

EGLSurface EglFSWindow::surface() const
{
    return m_eglSurface;
}

QRect EglFSWindow::geometry() const
{
    return QPlatformWindow::geometry();
}

void EglFSWindow::setGeometry(const QRect &rect)
{
    const auto oldGeometry = geometry();

    QPlatformWindow::setGeometry(rect);

    if (window()->isVisible() && rect.isValid()) {
        // m_window->resize(rect.size());
        QWindowSystemInterface::handleGeometryChange(window(), geometry());
    }

    if (isExposed() && oldGeometry.size() != rect.size())
        QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(), rect.size()));
}

void EglFSWindow::setVisible(bool visible)
{
    QPlatformWindow::setVisible(visible);
}

void EglFSWindow::requestActivateWindow()
{
    auto *w = window();
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    QWindowSystemInterface::handleFocusWindowChanged(w);
#else
    QWindowSystemInterface::handleWindowActivated(w);
#endif
    QWindowSystemInterface::handleExposeEvent(w, QRect(QPoint(0, 0), w->geometry().size()));
}

void EglFSWindow::raise()
{
    qCWarning(gLcEglFS, "Raising a window is not supported");
}

void EglFSWindow::lower()
{
    qCWarning(gLcEglFS, "Lowering a window is not supported");
}

void EglFSWindow::setOpacity(qreal level)
{
    Q_UNUSED(level)
    qCWarning(gLcEglFS, "Setting opacity of a window is not supported");
}

void EglFSWindow::setMask(const QRegion &region)
{
    Q_UNUSED(region)
    qCWarning(gLcEglFS, "Setting mask of a window is not supported");
}

QSurfaceFormat EglFSWindow::format() const
{
    return m_format;
}

void EglFSWindow::invalidateSurface()
{
}

void EglFSWindow::resetSurface()
{
}

bool EglFSWindow::isRaster() const
{
    return false;
}

void EglFSWindow::destroy()
{
    EGLDisplay eglDisplay = m_integration->display();

    if (m_eglSurface != EGL_NO_SURFACE) {
        eglDestroySurface(eglDisplay, m_eglSurface);
        m_eglSurface = EGL_NO_SURFACE;
    }

    if (m_eglWindow) {
        Aurora::Platform::auroraDeviceIntegration()->destroyNativeWindow(m_eglWindow);
        m_eglWindow = 0;
    }

    if (m_window)
        m_window->destroy();

    m_created = false;
}

bool EglFSWindow::createSurface()
{
    EGLDisplay eglDisplay = m_integration->display();
    QSurfaceFormat platformFormat = Aurora::Platform::auroraDeviceIntegration()->surfaceFormatFor(
            window()->requestedFormat());

    EglFSConfigChooser chooser(eglDisplay);
    chooser.setSurfaceType(Aurora::Platform::auroraDeviceIntegration()->surfaceType());
    chooser.setSurfaceFormat(platformFormat);
    m_config = chooser.chooseConfig();

    m_format = q_glFormatFromConfig(eglDisplay, m_config, platformFormat);

    const auto surfaceSize = screen()->geometry().size();
    m_eglWindow = Aurora::Platform::auroraDeviceIntegration()->createNativeWindow(
            m_window.data(), surfaceSize, m_format);
    if (!m_eglWindow) {
        qCCritical(gLcEglFS, "Could not create native window for size %dx%d", surfaceSize.width(),
                   surfaceSize.height());
        return false;
    }

    m_eglSurface = eglCreateWindowSurface(eglDisplay, m_config, m_eglWindow, nullptr);
    if (Q_UNLIKELY(m_eglSurface == EGL_NO_SURFACE)) {
        EGLint error = eglGetError();
        qCCritical(gLcEglFS, "Could not create EGL surface: error 0x%x\n", error);
        return false;
    }

    return true;
}
