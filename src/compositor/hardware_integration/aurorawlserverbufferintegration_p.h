// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDSERVERBUFFERINTEGRATION_H
#define AURORA_COMPOSITOR_WAYLANDSERVERBUFFERINTEGRATION_H

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

#include <QtCore/QSize>
#include <QtGui/qopengl.h>

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>

struct wl_client;
struct wl_resource;

class QOpenGLContext;
class QOpenGLTexture;
class QImage;

namespace Aurora {

namespace Compositor {

class WaylandCompositor;

namespace Internal {
class Display;

class LIRIAURORACOMPOSITOR_EXPORT ServerBuffer
{
public:
    enum Format {
        RGBA32,
        A8,
        Custom
    };

    ServerBuffer(const QSize &size, ServerBuffer::Format format);
    virtual ~ServerBuffer();

    virtual struct ::wl_resource *resourceForClient(struct ::wl_client *) = 0;
    virtual bool bufferInUse() { return true; }

    virtual QOpenGLTexture *toOpenGlTexture() = 0;
    virtual void releaseOpenGlTexture() {}

    virtual bool isYInverted() const;

    QSize size() const;
    Format format() const;
protected:
    QSize m_size;
    Format m_format;
};

class LIRIAURORACOMPOSITOR_EXPORT ServerBufferIntegration
{
public:
    ServerBufferIntegration();
    virtual ~ServerBufferIntegration();

    virtual bool initializeHardware(WaylandCompositor *);

    virtual bool supportsFormat(ServerBuffer::Format format) const = 0;
    virtual ServerBuffer *createServerBufferFromImage(const QImage &qimage, ServerBuffer::Format format) = 0;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual ServerBuffer *createServerBufferFromData(QByteArrayView view, const QSize &size,
                                                     uint glInternalFormat)
    {
        Q_UNUSED(view);
        Q_UNUSED(size);
        Q_UNUSED(glInternalFormat);
        return nullptr;
    }
#else
    virtual ServerBuffer *createServerBufferFromData(const QByteArray &data, const QSize &size,
                                                     uint glInternalFormat)
    {
        Q_UNUSED(data);
        Q_UNUSED(size);
        Q_UNUSED(glInternalFormat);
        return nullptr;
    }
#endif
};

}

} // namespace Compositor

} // namespace Aurora

#endif //QWAYLANDSERVERBUFFERINTEGRATION_H
