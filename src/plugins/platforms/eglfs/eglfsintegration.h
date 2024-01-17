// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWindow>

#include <qpa/qplatformintegration.h>
#include <qpa/qplatformnativeinterface.h>

#include <LiriAuroraPlatform/DeviceIntegration>
#include <LiriAuroraPlatform/Output>

#include "eglfscontext.h"

class QEventLoop;

class EglFSInputManager;

struct EglFSIntegrationOptions
{
    bool disableInputHandlers = false;
};

class EglFSIntegration
    : public QPlatformIntegration
    , public QPlatformNativeInterface
#ifndef QT_NO_OPENGL
    , public QNativeInterface::Private::QEGLIntegration
#endif
{
public:
    EglFSIntegration(const EglFSIntegrationOptions &options);
    ~EglFSIntegration();

    void initialize() override;
    void destroy() override;

    EGLDisplay display() const;

    QAbstractEventDispatcher *createEventDispatcher() const override;
    QPlatformFontDatabase *fontDatabase() const override;
    QPlatformServices *services() const override;
    QPlatformInputContext *inputContext() const override;
    QPlatformTheme *createPlatformTheme(const QString &name) const override;

    QPlatformWindow *createPlatformWindow(QWindow *window) const override;
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const override;
#ifndef QT_NO_OPENGL
    QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const override;
    QOpenGLContext *createOpenGLContext(EGLContext context, EGLDisplay display,
                                        QOpenGLContext *shareContext) const override;
    QPlatformOffscreenSurface *
    createPlatformOffscreenSurface(QOffscreenSurface *surface) const override;
#endif
#if QT_CONFIG(vulkan)
    QPlatformVulkanInstance *createPlatformVulkanInstance(QVulkanInstance *instance) const override;
#endif
    bool hasCapability(QPlatformIntegration::Capability cap) const override;

    QPlatformNativeInterface *nativeInterface() const override;

    // QPlatformNativeInterface
    void *nativeResourceForIntegration(const QByteArray &resource) override;
    void *nativeResourceForScreen(const QByteArray &resource, QScreen *screen) override;
    void *nativeResourceForWindow(const QByteArray &resource, QWindow *window) override;
#ifndef QT_NO_OPENGL
    void *nativeResourceForContext(const QByteArray &resource, QOpenGLContext *context) override;
#endif
    NativeResourceForContextFunction
    nativeResourceFunctionForContext(const QByteArray &resource) override;

    QFunctionPointer platformFunction(const QByteArray &function) const override;

    QVariant styleHint(QPlatformIntegration::StyleHint hint) const override;

private:
    EglFSIntegrationOptions m_options;
    QEventLoop *m_loop = nullptr;
    bool m_ready = false;
    EGLDisplay m_display = EGL_NO_DISPLAY;
    QPlatformInputContext *m_inputContext = nullptr;
    QScopedPointer<QPlatformFontDatabase> m_fontDb;
    QScopedPointer<QPlatformServices> m_services;
    QScopedPointer<EglFSInputManager> m_inputManager;

private slots:
    void handleIntegrationStatusChanged(Aurora::Platform::DeviceIntegration::Status status);
    void handleOutputAdded(Aurora::Platform::Output *output);
    void handleOutputRemoved(Aurora::Platform::Output *output);
};
