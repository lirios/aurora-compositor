// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef WAYLANDEGLSTREAMCONTROLLER_H
#define WAYLANDEGLSTREAMCONTROLLER_H

#include "aurora-server-wl-eglstream-controller.h"

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>
#include <LiriAuroraCompositor/private/aurorawlclientbufferintegration_p.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtOpenGL/QOpenGLTexture>
#else
#include <QtGui/QOpenGLTexture>
#endif
#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QSize>
#include <QtCore/QTextStream>

#include <EGL/egl.h>
#include <EGL/eglext.h>


namespace Aurora {

namespace Compositor {

class WaylandCompositor;
class WaylandResource;
class WaylandEglStreamClientBufferIntegration;

class WaylandEglStreamController : public PrivateServer::wl_eglstream_controller
{
public:
    explicit WaylandEglStreamController(wl_display *display, WaylandEglStreamClientBufferIntegration *clientBufferIntegration);

protected:
    void eglstream_controller_attach_eglstream_consumer(Resource *resource, struct ::wl_resource *wl_surface, struct ::wl_resource *wl_buffer) override;

private:
    WaylandEglStreamClientBufferIntegration *m_clientBufferIntegration;
};


} // namespace Compositor

} // namespace Aurora

#endif // WAYLANDEGLSTREAMCONTROLLER_H
