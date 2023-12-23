// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/private/aurorawlserverbufferintegration_p.h>

#include "aurora-server-shm-emulation-server-buffer.h"

#include <QtGui/QImage>
#include <QtGui/QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/QGuiApplication>

#include <LiriAuroraCompositor/aurorawaylandcompositor.h>
#include <LiriAuroraCompositor/private/aurora-server-server-buffer-extension.h>

class QSharedMemory;

namespace Aurora {

namespace Compositor {

class ShmServerBufferIntegration;

class ShmServerBuffer : public Internal::ServerBuffer, public PrivateServer::qt_server_buffer
{
public:
    ShmServerBuffer(ShmServerBufferIntegration *integration, const QImage &qimage, Internal::ServerBuffer::Format format);
    ~ShmServerBuffer() override;

    struct ::wl_resource *resourceForClient(struct ::wl_client *) override;
    bool bufferInUse() override;
    QOpenGLTexture *toOpenGlTexture() override;

private:
    ShmServerBufferIntegration *m_integration = nullptr;

    QSharedMemory *m_shm = nullptr;
    int m_width;
    int m_height;
    int m_bpl;
    QOpenGLTexture *m_texture = nullptr;
    PrivateServer::qt_shm_emulation_server_buffer::format m_shm_format;
};

class ShmServerBufferIntegration :
    public Internal::ServerBufferIntegration,
    public PrivateServer::qt_shm_emulation_server_buffer
{
public:
    ShmServerBufferIntegration();
    ~ShmServerBufferIntegration() override;

    bool initializeHardware(WaylandCompositor *) override;

    bool supportsFormat(Internal::ServerBuffer::Format format) const override;
    Internal::ServerBuffer *createServerBufferFromImage(const QImage &qimage, Internal::ServerBuffer::Format format) override;


private:
};

} // namespace Compositor

} // namespace Aurora

