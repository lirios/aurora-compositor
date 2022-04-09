/****************************************************************************
**
** Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef AURORA_COMPOSITOR_WAYLANDBUFFERREF_H
#define AURORA_COMPOSITOR_WAYLANDBUFFERREF_H

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <QImage>

#if QT_CONFIG(opengl)
#include <QtGui/qopengl.h>
#endif

#include <LiriAuroraCompositor/WaylandSurface>

struct wl_resource;

class QOpenGLTexture;

namespace Aurora {

namespace Compositor {

namespace QtWayland
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
    explicit WaylandBufferRef(QtWayland::ClientBuffer *buffer);
    QtWayland::ClientBuffer *buffer() const;
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

#endif
