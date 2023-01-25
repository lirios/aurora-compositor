// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Aurora API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef WAYLANDEGLINTEGRATION_H
#define WAYLANDEGLINTEGRATION_H

#include <LiriAuroraCompositor/private/aurorawlclientbufferintegration_p.h>
#include <QtCore/QScopedPointer>
#include <LiriAuroraCompositor/private/aurorawlclientbuffer_p.h>

namespace Aurora {

namespace Compositor {

class WaylandEglClientBufferIntegrationPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandEglClientBufferIntegration : public Internal::ClientBufferIntegration
{
    Q_DECLARE_PRIVATE(WaylandEglClientBufferIntegration)
public:
    WaylandEglClientBufferIntegration();
    ~WaylandEglClientBufferIntegration() override;

    void initializeHardware(struct ::wl_display *display) override;

    Internal::ClientBuffer *createBufferFor(wl_resource *buffer) override;

private:
    Q_DISABLE_COPY(WaylandEglClientBufferIntegration)
    QScopedPointer<WaylandEglClientBufferIntegrationPrivate> d_ptr;
};

struct BufferState;

class LIRIAURORACOMPOSITOR_EXPORT WaylandEglClientBuffer : public Internal::ClientBuffer
{
public:
    WaylandEglClientBuffer(WaylandEglClientBufferIntegration* integration, wl_resource *bufferResource);
    ~WaylandEglClientBuffer() override;

    WaylandBufferRef::BufferFormatEgl bufferFormatEgl() const override;
    QSize size() const override;
    WaylandSurface::Origin origin() const override;
    quintptr lockNativeBuffer() override;
    void unlockNativeBuffer(quintptr native_buffer) const override;
    QOpenGLTexture *toOpenGlTexture(int plane) override;
    void setCommitted(QRegion &damage) override;
    bool isProtected() override;

private:
    friend class WaylandEglClientBufferIntegration;
    friend class WaylandEglClientBufferIntegrationPrivate;

    BufferState *d = nullptr;
    WaylandEglClientBufferIntegration *m_integration = nullptr;
};

} // namespace Compositor

} // namespace Aurora

#endif // WAYLANDEGLINTEGRATION_H
