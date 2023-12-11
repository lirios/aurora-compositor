// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/private/aurorawlserverbufferintegration_p.h>

#include "aurora-server-qt-vulkan-server-buffer-unstable-v1.h"

#include <QtGui/QImage>
#include <QtGui/QWindow>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/QGuiApplication>

#include <LiriAuroraCompositor/aurorawaylandcompositor.h>
#include <LiriAuroraCompositor/private/aurora-server-server-buffer-extension.h>

namespace Aurora {

namespace Compositor {

class VulkanServerBufferIntegration;
class VulkanWrapper;
struct VulkanImageWrapper;

class VulkanServerBuffer : public Internal::ServerBuffer, public PrivateServer::qt_server_buffer
{
public:
    VulkanServerBuffer(VulkanServerBufferIntegration *integration, const QImage &qimage, Internal::ServerBuffer::Format format);
    VulkanServerBuffer(VulkanServerBufferIntegration *integration, VulkanImageWrapper *vImage, uint glInternalFormat, const QSize &size);
    ~VulkanServerBuffer() override;

    struct ::wl_resource *resourceForClient(struct ::wl_client *) override;
    bool bufferInUse() override;
    QOpenGLTexture *toOpenGlTexture() override;
    void releaseOpenGlTexture() override;

protected:
    void server_buffer_release(Resource *resource) override;

private:
    VulkanServerBufferIntegration *m_integration = nullptr;

    int m_width;
    int m_height;
    int m_memorySize;
    int m_fd = -1;
    VulkanImageWrapper *m_vImage = nullptr;
    QOpenGLTexture *m_texture = nullptr;
    uint m_glInternalFormat;
    GLuint m_memoryObject;
};

class VulkanServerBufferIntegration :
    public Internal::ServerBufferIntegration,
    public PrivateServer::zqt_vulkan_server_buffer_v1
{
public:
    VulkanServerBufferIntegration();
    ~VulkanServerBufferIntegration() override;

    VulkanWrapper *vulkanWrapper() const { return m_vulkanWrapper; }

    bool initializeHardware(WaylandCompositor *) override;

    bool supportsFormat(Internal::ServerBuffer::Format format) const override;
    Internal::ServerBuffer *createServerBufferFromImage(const QImage &qimage, Internal::ServerBuffer::Format format) override;
    Internal::ServerBuffer *createServerBufferFromData(QByteArrayView view, const QSize &size,
                                                        uint glInternalFormat) override;

private:
    VulkanWrapper *m_vulkanWrapper = nullptr;
};

} // namespace Compositor

} // namespace Aurora

