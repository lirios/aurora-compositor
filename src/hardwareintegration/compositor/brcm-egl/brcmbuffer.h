// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>
#include <LiriAuroraCompositor/private/aurorawaylandutils_p.h>

#include <QtCore/QList>
#include <QtCore/QSize>

#include <EGL/egl.h>

namespace Aurora {

namespace Compositor {

class BrcmBuffer : public PrivateServer::wl_buffer
{
public:
    BrcmBuffer(struct ::wl_client *client, uint32_t id, const QSize &size, EGLint *data, size_t count);
    ~BrcmBuffer();

    bool isYInverted() const { return m_invertedY; }
    void setInvertedY(bool inverted) { m_invertedY = inverted; }

    EGLint *handle() { return m_handle.data(); }

    QSize size() { return m_size; }

    static BrcmBuffer *fromResource(struct ::wl_resource *resource) { return Internal::fromResource<BrcmBuffer *>(resource); }

protected:
    void buffer_destroy_resource(Resource *resource) override;
    void buffer_destroy(Resource *resource) override;

private:
    QList<EGLint> m_handle;
    bool m_invertedY = false;
    QSize m_size;
};

} // namespace Compositor

} // namespace Aurora

