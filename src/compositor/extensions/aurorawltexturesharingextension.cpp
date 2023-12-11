// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawltexturesharingextension_p.h"

#include <LiriAuroraCompositor/WaylandSurface>

#include <QDebug>

#include <QQuickWindow>

#include <QPainter>
#include <QPen>
#include <QTimer>

#include <QtGui/private/qtexturefilereader_p.h>

#include <QtOpenGL/QOpenGLTexture>
#include <QtGui/QImageReader>

#include <QtQuick/QSGTexture>
#include <QQmlContext>
#include <QThread>

namespace Aurora {

namespace Compositor {

class SharedTextureFactory : public QQuickTextureFactory
{
public:
    SharedTextureFactory(const Internal::ServerBuffer *buffer)
        : m_buffer(buffer)
    {
    }

    ~SharedTextureFactory() override
    {
        if (m_buffer && !QCoreApplication::closingDown())
            const_cast<Internal::ServerBuffer*>(m_buffer)->releaseOpenGlTexture();
    }

   QSize textureSize() const override
    {
        return m_buffer ? m_buffer->size() : QSize();
    }

    int textureByteCount() const override
    {
        return m_buffer ? (m_buffer->size().width() * m_buffer->size().height() * 4) : 0;
    }

    QSGTexture *createTexture(QQuickWindow *window) const override
    {
        if (m_buffer != nullptr) {
            QOpenGLTexture *texture = const_cast<Internal::ServerBuffer *>(m_buffer)->toOpenGlTexture();
            return QNativeInterface::QSGOpenGLTexture::fromNative(texture->textureId(),
                                                                   window,
                                                                   m_buffer->size(),
                                                                   QQuickWindow::TextureHasAlphaChannel);
        }

        return nullptr;
    }

private:
    const Internal::ServerBuffer *m_buffer = nullptr;
};

class SharedTextureImageResponse : public QQuickImageResponse
{
    Q_OBJECT
public:
    SharedTextureImageResponse(WaylandTextureSharingExtension *extension, const QString &id)
        : m_id(id)
    {
        if (extension)
            doRequest(extension);
    }

    void doRequest(WaylandTextureSharingExtension *extension)
    {
        m_extension = extension;
        connect(extension, &WaylandTextureSharingExtension::bufferResult, this, &SharedTextureImageResponse::doResponse);
        QMetaObject::invokeMethod(extension, [this] { m_extension->requestBuffer(m_id); }, Qt::AutoConnection);
    }

    QQuickTextureFactory *textureFactory() const override
    {
        if (m_buffer) {
//            qDebug() << "Creating shared buffer texture for" << m_id;
            return new SharedTextureFactory(m_buffer);
        }
//        qDebug() << "Shared buffer NOT found for" << m_id;
        m_errorString = QLatin1String("Shared buffer not found");
        return nullptr;
    }

    QString errorString() const override
    {
        return m_errorString;
    }

public Q_SLOTS:
    void doResponse(const QString &key, Internal::ServerBuffer *buffer)
    {
        if (key != m_id)
            return; //somebody else's texture

        m_buffer = buffer;

        if (m_extension)
            disconnect(m_extension, &WaylandTextureSharingExtension::bufferResult, this, &SharedTextureImageResponse::doResponse);

        emit finished();
    }

private:
    QString m_id;
    WaylandTextureSharingExtension *m_extension = nullptr;
    mutable QString m_errorString;
    Internal::ServerBuffer *m_buffer = nullptr;
};

WaylandSharedTextureProvider::WaylandSharedTextureProvider()
{
}

WaylandSharedTextureProvider::~WaylandSharedTextureProvider()
{
}

QQuickImageResponse *WaylandSharedTextureProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);

//    qDebug() << "Provider: got request for" << id;

    auto *extension = WaylandTextureSharingExtension::self();
    auto *response = new SharedTextureImageResponse(extension, id);
    if (!extension)
        m_pendingResponses << response;

    return response;
}

void WaylandSharedTextureProvider::setExtensionReady(WaylandTextureSharingExtension *extension)
{
    for (auto *response : std::as_const(m_pendingResponses))
        response->doRequest(extension);
    m_pendingResponses.clear();
    m_pendingResponses.squeeze();
}

WaylandTextureSharingExtension *WaylandTextureSharingExtension::s_self = nullptr; // theoretical race conditions, but OK as long as we don't delete it while we are running

WaylandTextureSharingExtension::WaylandTextureSharingExtension()
{
    s_self = this;
}

WaylandTextureSharingExtension::WaylandTextureSharingExtension(WaylandCompositor *compositor)
    :WaylandCompositorExtensionTemplate(compositor)
{
    s_self = this;
}

WaylandTextureSharingExtension::~WaylandTextureSharingExtension()
{
    //qDebug() << Q_FUNC_INFO;
    //dumpBufferInfo();

    for (auto b : m_server_buffers)
        delete b.buffer;

    if (s_self == this)
        s_self = nullptr;
}

void WaylandTextureSharingExtension::setImageSearchPath(const QString &path)
{
    m_image_dirs = path.split(QLatin1Char(';'));

    for (auto it = m_image_dirs.begin(); it != m_image_dirs.end(); ++it)
        if (!(*it).endsWith(QLatin1Char('/')))
            (*it) += QLatin1Char('/');
}

void WaylandTextureSharingExtension::initialize()
{
    WaylandCompositorExtensionTemplate::initialize();
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    init(compositor->display(), 1);

    QString image_search_path = qEnvironmentVariable("QT_WAYLAND_SHAREDTEXTURE_SEARCH_PATH");
    if (!image_search_path.isEmpty())
        setImageSearchPath(image_search_path);

    if (m_image_dirs.isEmpty())
        m_image_dirs << QLatin1String(":/") << QLatin1String("./");

    auto suffixes = QTextureFileReader::supportedFileFormats();
    suffixes.append(QImageReader::supportedImageFormats());
    for (auto ext : std::as_const(suffixes))
        m_image_suffixes << QLatin1Char('.') + QString::fromLatin1(ext);

    //qDebug() << "m_image_suffixes" << m_image_suffixes << "m_image_dirs" << m_image_dirs;

    auto *ctx = QQmlEngine::contextForObject(this);
    if (ctx) {
        QQmlEngine *engine = ctx->engine();
        if (engine) {
            auto *provider = static_cast<WaylandSharedTextureProvider*>(engine->imageProvider(QLatin1String("wlshared")));
            if (provider)
                provider->setExtensionReady(this);
        }
    }
}

QString WaylandTextureSharingExtension::getExistingFilePath(const QString &key) const
{
    // The default search path blocks absolute pathnames, but this does not prevent relative
    // paths containing '../'. We handle that here, at the price of also blocking directory
    // names ending with two or more dots.

    if (key.contains(QLatin1String("../")))
        return QString();

    for (auto dir : std::as_const(m_image_dirs)) {
        QString path = dir + key;
        if (QFileInfo::exists(path))
            return path;
    }

    for (auto dir : std::as_const(m_image_dirs)) {
        for (auto ext : m_image_suffixes) {
            QString fp = dir + key + ext;
            //qDebug() << "trying" << fp;
            if (QFileInfo::exists(fp))
                return fp;
        }
    }
    return QString();
}

Internal::ServerBuffer *WaylandTextureSharingExtension::getBuffer(const QString &key)
{
    if (!initServerBufferIntegration())
        return nullptr;

//qDebug() << "getBuffer" << key;

    Internal::ServerBuffer *buffer = nullptr;

    if ((buffer = m_server_buffers.value(key).buffer))
        return buffer;

    QByteArray pixelData;
    QSize size;
    uint glInternalFormat = GL_NONE;

    if (customPixelData(key, &pixelData, &size, &glInternalFormat)) {
        if (!pixelData.isEmpty()) {
            buffer = m_server_buffer_integration->createServerBufferFromData(pixelData, size, glInternalFormat);
            if (!buffer)
                qWarning() << "WaylandTextureSharingExtension: could not create buffer from custom data for key:" << key;
        }
    } else {
        QString pathName = getExistingFilePath(key);
        //qDebug() << "pathName" << pathName;
        if (pathName.isEmpty())
            return nullptr;

        buffer = getCompressedBuffer(pathName);
        //qDebug() << "getCompressedBuffer" << buffer;

        if (!buffer) {
            QImage img(pathName);
            if (!img.isNull()) {
                img = img.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
                buffer = m_server_buffer_integration->createServerBufferFromImage(img, Internal::ServerBuffer::RGBA32);
            }
            //qDebug() << "createServerBufferFromImage" << buffer;
        }
    }
    if (buffer)
        m_server_buffers.insert(key, BufferInfo(buffer));

    //qDebug() << ">>>>" << key << buffer;

    return buffer;
}

// Compositor requesting image for its own UI
void WaylandTextureSharingExtension::requestBuffer(const QString &key)
{
    //qDebug() << "requestBuffer" << key;

    if (thread() != QThread::currentThread())
        qWarning("WaylandTextureSharingExtension::requestBuffer() called from outside main thread: possible race condition");

    auto *buffer = getBuffer(key);

    if (buffer)
        m_server_buffers[key].usedLocally = true;

    //dumpBufferInfo();

    emit bufferResult(key, buffer);
}

void WaylandTextureSharingExtension::zqt_texture_sharing_v1_request_image(Resource *resource, const QString &key)
{
    //qDebug() << "texture_sharing_request_image" << key;
    auto *buffer = getBuffer(key);
    if (buffer) {
        struct ::wl_client *client = resource->client();
        struct ::wl_resource *buffer_resource = buffer->resourceForClient(client);
        //qDebug() << "          server_buffer resource" << buffer_resource;
        if (buffer_resource)
            send_provide_buffer(resource->handle, buffer_resource, key);
        else
            qWarning() << "WaylandTextureSharingExtension: no buffer resource for client";
    } else {
        send_image_failed(resource->handle, key, QString());
    }
    //dumpBufferInfo();
}

void WaylandTextureSharingExtension::zqt_texture_sharing_v1_abandon_image(Resource *resource, const QString &key)
{
    Q_UNUSED(resource);
    Q_UNUSED(key);
//    qDebug() << Q_FUNC_INFO << resource << key;
    QTimer::singleShot(100, this, &WaylandTextureSharingExtension::cleanupBuffers);
}

// A client has disconnected
void WaylandTextureSharingExtension::zqt_texture_sharing_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
//    qDebug() << "texture_sharing_destroy_resource" << resource->handle << resource->handle->object.id << "client" << resource->client();
//    dumpBufferInfo();
    QTimer::singleShot(1000, this, &WaylandTextureSharingExtension::cleanupBuffers);
}

bool WaylandTextureSharingExtension::initServerBufferIntegration()
{
    if (!m_server_buffer_integration) {
        WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());

        m_server_buffer_integration = WaylandCompositorPrivate::get(compositor)->serverBufferIntegration();
        if (!m_server_buffer_integration) {
            qWarning("WaylandTextureSharingExtension initialization failed: No Server Buffer Integration");
            if (qEnvironmentVariableIsEmpty("QT_WAYLAND_SERVER_BUFFER_INTEGRATION"))
                qWarning("Set the environment variable 'QT_WAYLAND_SERVER_BUFFER_INTEGRATION' to specify.");
            return false;
        }
    }
    return true;
}

Internal::ServerBuffer *WaylandTextureSharingExtension::getCompressedBuffer(const QString &pathName)
{
    QFile f(pathName);
    if (!f.open(QIODevice::ReadOnly))
        return nullptr;

    QTextureFileReader r(&f, pathName);

    if (!r.canRead())
        return nullptr;

    QTextureFileData td(r.read());

    //qDebug() << "WaylandTextureSharingExtension: reading compressed texture data" << td;

    if (!td.isValid()) {
        qWarning() << "VulkanServerBufferIntegration:" << pathName << "not valid compressed texture";
        return nullptr;
    }

    return m_server_buffer_integration->createServerBufferFromData(td.getDataView(), td.size(),
                                                                   td.glInternalFormat());
}

void WaylandTextureSharingExtension::cleanupBuffers()
{
    for (auto it = m_server_buffers.begin(); it != m_server_buffers.end(); ) {
        auto *buffer = it.value().buffer;
        if (!it.value().usedLocally && !buffer->bufferInUse()) {
            //qDebug() << "deleting buffer for" << it.key();
            it = m_server_buffers.erase(it);
            delete buffer;
        } else {
            ++it;
        }
    }
    //dumpBufferInfo();
}

void WaylandTextureSharingExtension::dumpBufferInfo()
{
    qDebug() << "shared buffers:" << m_server_buffers.size();
    for (auto it = m_server_buffers.cbegin(); it != m_server_buffers.cend(); ++it)
        qDebug() << "    " << it.key() << ":" << it.value().buffer << "in use" << it.value().buffer->bufferInUse() << "usedLocally" << it.value().usedLocally ;
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawltexturesharingextension_p.cpp"

#include "aurorawltexturesharingextension.moc"
