// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <QtGui/QImage>

#if QT_CONFIG(opengl)
#include <QtGui/qopengl.h>
#endif

#include <LiriAuroraCompositor/WaylandSurface>

struct wl_resource;

class QOpenGLTexture;

namespace Aurora {

namespace Compositor {

namespace Internal
{
    class ClientBuffer;
}

class LIRIAURORACOMPOSITOR_EXPORT WaylandBufferRef
{
public:
    WaylandBufferRef();
    WaylandBufferRef(const WaylandBufferRef &ref);
    ~WaylandBufferRef();

    WaylandBufferRef &operator=(const WaylandBufferRef &ref);
    bool isNull() const;
    bool hasBuffer() const;
    bool hasContent() const;
    bool hasProtectedContent() const;
    bool isDestroyed() const;

    struct wl_resource *wl_buffer() const;

    QSize size() const;
    WaylandSurface::Origin origin() const;

    enum BufferType {
        BufferType_Null,
        BufferType_SharedMemory,
        BufferType_Egl
    };

    enum BufferFormatEgl {
        BufferFormatEgl_Null,
        BufferFormatEgl_RGB,
        BufferFormatEgl_RGBA,
        BufferFormatEgl_EXTERNAL_OES,
        BufferFormatEgl_Y_U_V,
        BufferFormatEgl_Y_UV,
        BufferFormatEgl_Y_XUXV
    };

    BufferType bufferType() const;
    BufferFormatEgl bufferFormatEgl() const;

    bool isSharedMemory() const;
    QImage image() const;

#if QT_CONFIG(opengl)
    QOpenGLTexture *toOpenGLTexture(int plane = 0) const;
#endif

    quintptr lockNativeBuffer();
    void unlockNativeBuffer(quintptr handle);

private:
    explicit WaylandBufferRef(Internal::ClientBuffer *buffer);
    Internal::ClientBuffer *buffer() const;
    class WaylandBufferRefPrivate *const d;
    friend class WaylandBufferRefPrivate;
    friend class WaylandSurfacePrivate;

    friend LIRIAURORACOMPOSITOR_EXPORT
    bool operator==(const WaylandBufferRef &lhs, const WaylandBufferRef &rhs) noexcept;
    friend inline
    bool operator!=(const WaylandBufferRef &lhs, const WaylandBufferRef &rhs) noexcept
    { return !(lhs == rhs); }
};

} // namespace Compositor

} // namespace Aurora

