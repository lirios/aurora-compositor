// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include "linuxdmabuf.h"

#include <LiriAuroraCompositor/private/aurorawlclientbufferintegration_p.h>
#include <LiriAuroraCompositor/private/aurorawlclientbuffer_p.h>
#include <LiriAuroraCompositor/private/aurora-server-wayland.h>
#include <QtCore/QMutex>

#include <drm_fourcc.h>

namespace Aurora {

namespace Compositor {

typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYWAYLANDBUFFERWL_compat) (EGLDisplay dpy, struct wl_resource *buffer, EGLint attribute, EGLint *value);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYDMABUFFORMATSEXTPROC) (EGLDisplay dpy, EGLint max_formats, EGLint *formats, EGLint *num_formats);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYDMABUFMODIFIERSEXTPROC) (EGLDisplay dpy, EGLint format, EGLint max_modifiers, EGLuint64KHR *modifiers, EGLBoolean *external_only, EGLint *num_modifiers);

class LinuxDmabufClientBufferIntegrationPrivate;
class LinuxDmabufParams;
class LinuxDmabufClientBuffer;

// buffer conversion definitions to import YUV buffers
struct YuvPlaneConversion {
    EGLint format = DRM_FORMAT_YUYV;
    EGLint widthDivisor = 1;
    EGLint heightDivisor = 1;
    EGLint planeIndex = 0;
};
struct YuvFormatConversion {
    uint32_t inputPlanes = 1;
    uint32_t outputPlanes = 1;
    struct YuvPlaneConversion plane[LinuxDmabufWlBuffer::MaxDmabufPlanes];
};

class LinuxDmabufClientBufferIntegration : public Internal::ClientBufferIntegration
{
public:
    LinuxDmabufClientBufferIntegration();
    ~LinuxDmabufClientBufferIntegration() override;

    void initializeHardware(struct ::wl_display *display) override;
    Internal::ClientBuffer *createBufferFor(wl_resource *resource) override;
    bool importBuffer(wl_resource *resource, LinuxDmabufWlBuffer *linuxDmabufBuffer);
    void removeBuffer(wl_resource *resource);
    void deleteImage(EGLImageKHR image);
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC gl_egl_image_target_texture_2d = nullptr;

private:
    Q_DISABLE_COPY(LinuxDmabufClientBufferIntegration)

    PFNEGLBINDWAYLANDDISPLAYWL egl_bind_wayland_display = nullptr;
    PFNEGLUNBINDWAYLANDDISPLAYWL egl_unbind_wayland_display = nullptr;
    PFNEGLCREATEIMAGEKHRPROC egl_create_image = nullptr;
    PFNEGLDESTROYIMAGEKHRPROC egl_destroy_image = nullptr;
    PFNEGLQUERYDMABUFMODIFIERSEXTPROC egl_query_dmabuf_modifiers_ext = nullptr;
    PFNEGLQUERYDMABUFFORMATSEXTPROC egl_query_dmabuf_formats_ext = nullptr;

    bool initSimpleTexture(LinuxDmabufWlBuffer *dmabufBuffer);
    bool initYuvTexture(LinuxDmabufWlBuffer *dmabufBuffer);
    QList<uint32_t> supportedDrmFormats();
    QList<uint64_t> supportedDrmModifiers(uint32_t format);

    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
    ::wl_display *m_wlDisplay = nullptr;
    bool m_displayBound = false;

    QHash<EGLint, YuvFormatConversion> m_yuvFormats;
    bool m_supportsDmabufModifiers = false;
    QHash<struct ::wl_resource *, LinuxDmabufWlBuffer *> m_importedBuffers;
    QScopedPointer<LinuxDmabuf> m_linuxDmabuf;
};

class LinuxDmabufClientBuffer : public Internal::ClientBuffer
{
public:
    ~LinuxDmabufClientBuffer() override;

    WaylandBufferRef::BufferFormatEgl bufferFormatEgl() const override;
    QSize size() const override;
    WaylandSurface::Origin origin() const override;
    QOpenGLTexture *toOpenGlTexture(int plane) override;

protected:
    void setDestroyed() override;

private:
    friend class LinuxDmabufClientBufferIntegration;
    friend class LinuxDmabufClientBufferIntegrationPrivate;

    LinuxDmabufClientBuffer(LinuxDmabufClientBufferIntegration* integration, wl_resource *bufferResource, LinuxDmabufWlBuffer *dmabufBuffer);

    LinuxDmabufWlBuffer *d = nullptr;
    LinuxDmabufClientBufferIntegration *m_integration = nullptr;
};

} // namespace Compositor

} // namespace Aurora

