// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "brcmeglintegration.h"
#include "brcmbuffer.h"
#include <LiriAuroraCompositor/aurorawaylandsurface.h>
#include <qpa/qplatformnativeinterface.h>
#include <QtGui/QGuiApplication>
#include <QtGui/QOpenGLContext>
#include <QOpenGLTexture>
#include <qpa/qplatformscreen.h>
#include <QtGui/QWindow>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <EGL/eglext_brcm.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

namespace Aurora {

namespace Compositor {

class BrcmEglIntegrationPrivate
{
public:
    BrcmEglIntegrationPrivate() = default;

    static BrcmEglIntegrationPrivate *get(BrcmEglIntegration *integration);

    EGLDisplay egl_display = EGL_NO_DISPLAY;
    bool valid = false;
    PFNEGLQUERYGLOBALIMAGEBRCMPROC eglQueryGlobalImageBRCM;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
    PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
};

BrcmEglIntegration::BrcmEglIntegration()
    : d_ptr(new BrcmEglIntegrationPrivate)
{
}

void BrcmEglIntegration::initializeHardware(struct ::wl_display *display)
{
    Q_D(BrcmEglIntegration);

    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();
    if (nativeInterface) {
        d->egl_display = nativeInterface->nativeResourceForIntegration("EglDisplay");
        if (!d->egl_display)
            qWarning("Failed to acquire EGL display from platform integration");

        d->eglQueryGlobalImageBRCM = (PFNEGLQUERYGLOBALIMAGEBRCMPROC) eglGetProcAddress("eglQueryGlobalImageBRCM");

        if (!d->eglQueryGlobalImageBRCM) {
            qWarning("Failed to resolve eglQueryGlobalImageBRCM");
            return;
        }

        d->glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");

        if (!d->glEGLImageTargetTexture2DOES) {
            qWarning("Failed to resolve glEGLImageTargetTexture2DOES");
            return;
        }

        d->eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");

        if (!d->eglCreateImageKHR) {
            qWarning("Failed to resolve eglCreateImageKHR");
            return;
        }

        d->eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");

        if (!d->eglDestroyImageKHR) {
            qWarning("Failed to resolve eglDestroyImageKHR");
            return;
        }
        d->valid = true;
        init(display, 1);
    }
}

Internal::ClientBuffer *BrcmEglIntegration::createBufferFor(wl_resource *buffer)
{
    if (wl_shm_buffer_get(buffer))
        return nullptr;
    return new BrcmEglClientBuffer(this, buffer);
}

BrcmEglIntegrationPrivate *BrcmEglIntegrationPrivate::get(BrcmEglIntegration *integration)
{
    return integration->d_ptr.data();
}

QOpenGLTexture *BrcmEglClientBuffer::toOpenGlTexture(int plane)
{
    Q_UNUSED(plane);

    auto d = BrcmEglIntegrationPrivate::get(m_integration);
    if (!d->valid) {
        qWarning("bindTextureToBuffer failed!");
        return nullptr;
    }

    BrcmBuffer *brcmBuffer = BrcmBuffer::fromResource(m_buffer);

    if (!d->eglQueryGlobalImageBRCM(brcmBuffer->handle(), brcmBuffer->handle() + 2)) {
        qWarning("eglQueryGlobalImageBRCM failed!");
        return nullptr;
    }

    EGLImageKHR image = d->eglCreateImageKHR(d->egl_display, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)brcmBuffer->handle(), NULL);
    if (image == EGL_NO_IMAGE_KHR)
        qWarning("eglCreateImageKHR() failed: %x\n", eglGetError());

    if (!m_texture) {
        m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        m_texture->create();
    }

    m_texture->bind();

    d->glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    d->eglDestroyImageKHR(d->egl_display, image);

    return m_texture;
}

void BrcmEglIntegration::brcm_bind_resource(Resource *)
{
}

void BrcmEglIntegration::brcm_create_buffer(Resource *resource, uint32_t id, int32_t width, int32_t height, wl_array *data)
{
    new BrcmBuffer(resource->client(), id, QSize(width, height), static_cast<EGLint *>(data->data), data->size / sizeof(EGLint));
}

BrcmEglClientBuffer::BrcmEglClientBuffer(BrcmEglIntegration *integration, wl_resource *buffer)
    : ClientBuffer(buffer)
    , m_integration(integration)
{
}

WaylandBufferRef::BufferFormatEgl BrcmEglClientBuffer::bufferFormatEgl() const
{
    return WaylandBufferRef::BufferFormatEgl_RGBA;
}

QSize BrcmEglClientBuffer::size() const
{
    BrcmBuffer *brcmBuffer = BrcmBuffer::fromResource(m_buffer);
    return brcmBuffer->size();
}

WaylandSurface::Origin BrcmEglClientBuffer::origin() const
{
    BrcmBuffer *brcmBuffer = BrcmBuffer::fromResource(m_buffer);
    return brcmBuffer->isYInverted() ? WaylandSurface::OriginTopLeft : WaylandSurface::OriginBottomLeft;
}


} // namespace Compositor

} // namespace Aurora
