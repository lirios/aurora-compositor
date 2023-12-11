// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

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

#include "wayland-util.h"

#include <QtCore/QMap>
#include <QtCore/QHash>

#include <LiriAuroraCompositor/WaylandCompositorExtensionTemplate>
#include <LiriAuroraCompositor/WaylandQuickExtension>
#include <LiriAuroraCompositor/WaylandCompositor>

#include <QQuickImageProvider>

#include <LiriAuroraCompositor/private/aurorawaylandcompositor_p.h>
#include <LiriAuroraCompositor/private/aurorawlserverbufferintegration_p.h>

#include <LiriAuroraCompositor/private/aurora-server-qt-texture-sharing-unstable-v1.h>

namespace Aurora {

namespace Compositor {

namespace Internal
{
    class ServerBufferIntegration;
}

class WaylandTextureSharingExtension;
class SharedTextureImageResponse;

class LIRIAURORACOMPOSITOR_EXPORT WaylandSharedTextureProvider : public QQuickAsyncImageProvider
{
public:
    WaylandSharedTextureProvider();
    ~WaylandSharedTextureProvider() override;

    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
    void setExtensionReady(WaylandTextureSharingExtension *extension);

private:
    QList<SharedTextureImageResponse*> m_pendingResponses;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandTextureSharingExtension
    : public WaylandCompositorExtensionTemplate<WaylandTextureSharingExtension>
    , public PrivateServer::zqt_texture_sharing_v1
{
    Q_OBJECT
    Q_PROPERTY(QString imageSearchPath WRITE setImageSearchPath)
public:
    WaylandTextureSharingExtension();
    WaylandTextureSharingExtension(WaylandCompositor *compositor);
    ~WaylandTextureSharingExtension() override;

    void initialize() override;

    void setImageSearchPath(const QString &path);

    static WaylandTextureSharingExtension *self() { return s_self; }

public Q_SLOTS:
    void requestBuffer(const QString &key);

Q_SIGNALS:
     void bufferResult(const QString &key, Internal::ServerBuffer *buffer);

protected Q_SLOTS:
    void cleanupBuffers();

protected:
    void zqt_texture_sharing_v1_request_image(Resource *resource, const QString &key) override;
    void zqt_texture_sharing_v1_abandon_image(Resource *resource, const QString &key) override;
    void zqt_texture_sharing_v1_destroy_resource(Resource *resource) override;

    virtual bool customPixelData(const QString &key, QByteArray *data, QSize *size, uint *glInternalFormat)
    {
        Q_UNUSED(key);
        Q_UNUSED(data);
        Q_UNUSED(size);
        Q_UNUSED(glInternalFormat);
        return false;
    }

private:
    Internal::ServerBuffer *getBuffer(const QString &key);
    bool initServerBufferIntegration();
    Internal::ServerBuffer *getCompressedBuffer(const QString &key);
    QString getExistingFilePath(const QString &key) const;
    void dumpBufferInfo();

    struct BufferInfo
    {
        BufferInfo(Internal::ServerBuffer *b = nullptr) : buffer(b) {}
        Internal::ServerBuffer *buffer = nullptr;
        bool usedLocally = false;
    };

    QStringList m_image_dirs;
    QStringList m_image_suffixes;
    QHash<QString, BufferInfo> m_server_buffers;
    Internal::ServerBufferIntegration *m_server_buffer_integration = nullptr;

    static WaylandTextureSharingExtension *s_self;
};

AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandTextureSharingExtension)

} // namespace Compositor

} // namespace Aurora

