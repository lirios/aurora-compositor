// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "waylandeglstreamcontroller.h"
#include "waylandeglstreamintegration.h"

#include <LiriAuroraCompositor/WaylandCompositor>

#include <unistd.h>

namespace Aurora {

namespace Compositor {



WaylandEglStreamController::WaylandEglStreamController(wl_display *display, WaylandEglStreamClientBufferIntegration *clientBufferIntegration)
    : wl_eglstream_controller(display, 1 /*version*/)
    , m_clientBufferIntegration(clientBufferIntegration)
{
}

void WaylandEglStreamController::eglstream_controller_attach_eglstream_consumer(Resource *resource, struct ::wl_resource *wl_surface, struct ::wl_resource *wl_buffer)
{
    Q_UNUSED(resource);
    m_clientBufferIntegration->attachEglStreamConsumer(wl_surface, wl_buffer);
}

} // namespace Compositor

} // namespace Aurora
