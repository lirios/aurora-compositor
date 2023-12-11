// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include "aurora-server-wl-eglstream-controller.h"

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>
#include <LiriAuroraCompositor/private/aurorawlclientbufferintegration_p.h>

#include <QtOpenGL/QOpenGLTexture>
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

