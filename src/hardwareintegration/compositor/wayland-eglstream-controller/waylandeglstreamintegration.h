// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#pragma once

#include <LiriAuroraCompositor/private/aurorawlclientbufferintegration_p.h>
#include <QtCore/QScopedPointer>
#include <LiriAuroraCompositor/private/aurorawlclientbuffer_p.h>

namespace Aurora {

namespace Compositor {

class WaylandEglStreamClientBufferIntegrationPrivate;

class WaylandEglStreamClientBufferIntegration : public Internal::ClientBufferIntegration
{
    Q_DECLARE_PRIVATE(WaylandEglStreamClientBufferIntegration)
public:
    WaylandEglStreamClientBufferIntegration();
    ~WaylandEglStreamClientBufferIntegration() override;

    void initializeHardware(struct ::wl_display *display) override;

    Internal::ClientBuffer *createBufferFor(wl_resource *buffer) override;

    void attachEglStreamConsumer(struct ::wl_resource *wl_surface, struct ::wl_resource *wl_buffer);

private:
    Q_DISABLE_COPY(WaylandEglStreamClientBufferIntegration)
    QScopedPointer<WaylandEglStreamClientBufferIntegrationPrivate> d_ptr;
};

struct BufferState;

class WaylandEglStreamClientBuffer : public Internal::ClientBuffer
{
public:
    ~WaylandEglStreamClientBuffer() override;

    WaylandBufferRef::BufferFormatEgl bufferFormatEgl() const override;
    QSize size() const override;
    WaylandSurface::Origin origin() const override;
    QOpenGLTexture *toOpenGlTexture(int plane) override;
    void setCommitted(QRegion &damage) override;

private:
    friend class WaylandEglStreamClientBufferIntegration;
    friend class WaylandEglStreamClientBufferIntegrationPrivate;

    WaylandEglStreamClientBuffer(WaylandEglStreamClientBufferIntegration* integration, wl_resource *bufferResource);

    BufferState *d = nullptr;
    WaylandEglStreamClientBufferIntegration *m_integration = nullptr;
};

} // namespace Compositor

} // namespace Aurora

