// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "shmserverbufferintegration.h"

#include <QtOpenGL/QOpenGLTexture>
#include <QtGui/QOpenGLContext>
#include <QtCore/QSharedMemory>

#include <QtCore/QDebug>

namespace Aurora {

namespace Compositor {

ShmServerBuffer::ShmServerBuffer(ShmServerBufferIntegration *integration, const QImage &qimage, Internal::ServerBuffer::Format format)
    : Internal::ServerBuffer(qimage.size(),format)
    , m_integration(integration)
    , m_width(qimage.width())
    , m_height(qimage.height())
    , m_bpl(qimage.bytesPerLine())
{
    m_format = format;
    switch (m_format) {
        case RGBA32:
            m_shm_format = PrivateServer::qt_shm_emulation_server_buffer::format_RGBA32;
            break;
        case A8:
            m_shm_format = PrivateServer::qt_shm_emulation_server_buffer::format_A8;
            break;
        default:
            qWarning("ShmServerBuffer: unsupported format");
            m_shm_format = PrivateServer::qt_shm_emulation_server_buffer::format_RGBA32;
            break;
    }

    QString key = QStringLiteral("qt_shm_emulation_%1").arg(QString::number(qimage.cacheKey()));
    // ### Use proper native keys the next time we can break protocol compatibility
    QT_IGNORE_DEPRECATIONS(m_shm = new QSharedMemory(key);)
    qsizetype shm_size = qimage.sizeInBytes();
    bool ok = m_shm->create(shm_size) && m_shm->lock();
    if (ok) {
        memcpy(m_shm->data(), qimage.constBits(), shm_size);
        m_shm->unlock();
    } else {
        qWarning() << "Could not create shared memory" << key << shm_size;
    }
}

ShmServerBuffer::~ShmServerBuffer()
{
    delete m_shm;
}

struct ::wl_resource *ShmServerBuffer::resourceForClient(struct ::wl_client *client)
{
    auto *bufferResource = resourceMap().value(client);
    if (!bufferResource) {
        auto integrationResource = m_integration->resourceMap().value(client);
        if (!integrationResource) {
            qWarning("ShmServerBuffer::resourceForClient: Trying to get resource for ServerBuffer. But client is not bound to the shm_emulation interface");
            return nullptr;
        }
        struct ::wl_resource *shm_integration_resource = integrationResource->handle;
        Resource *resource = add(client, 1);
        QT_IGNORE_DEPRECATIONS(const QString shmKey = m_shm->key();)
        m_integration->send_server_buffer_created(shm_integration_resource, resource->handle, shmKey, m_width, m_height, m_bpl, m_shm_format);
        return resource->handle;
    }
    return bufferResource->handle;
}

bool ShmServerBuffer::bufferInUse()
{
    return resourceMap().size() > 0;
}

QOpenGLTexture *ShmServerBuffer::toOpenGlTexture()
{
    if (!m_texture) {
        qWarning("ShmServerBuffer::toOpenGlTexture: no texture defined");
    }
    return m_texture;
}

ShmServerBufferIntegration::ShmServerBufferIntegration()
{
}

ShmServerBufferIntegration::~ShmServerBufferIntegration()
{
}

bool ShmServerBufferIntegration::initializeHardware(WaylandCompositor *compositor)
{
    Q_ASSERT(QGuiApplication::platformNativeInterface());

    PrivateServer::qt_shm_emulation_server_buffer::init(compositor->display(), 1);
    return true;
}

bool ShmServerBufferIntegration::supportsFormat(Internal::ServerBuffer::Format format) const
{
    switch (format) {
        case Internal::ServerBuffer::RGBA32:
            return true;
        case Internal::ServerBuffer::A8:
            return true;
        default:
            return false;
    }
}

Internal::ServerBuffer *ShmServerBufferIntegration::createServerBufferFromImage(const QImage &qimage, Internal::ServerBuffer::Format format)
{
    return new ShmServerBuffer(this, qimage, format);
}

} // namespace Compositor

} // namespace Aurora
