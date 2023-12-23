// Copyright (C) 2020 The Qt Company Ltd.
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

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/aurorawaylandsurface.h>
#include <LiriAuroraCompositor/aurorawaylandbufferref.h>
#include <QtCore/QSize>
#include <wayland-server-core.h>

class QOpenGLTexture;

namespace Aurora {

namespace Compositor {

class WaylandCompositor;

namespace Internal {
class Display;

class LIRIAURORACOMPOSITOR_EXPORT ClientBufferIntegration
{
public:
    ClientBufferIntegration();
    virtual ~ClientBufferIntegration() { }

    void setCompositor(WaylandCompositor *compositor) { m_compositor = compositor; }
    WaylandCompositor *compositor() const { return m_compositor; }

    virtual void initializeHardware(struct ::wl_display *display) = 0;

    virtual ClientBuffer *createBufferFor(struct ::wl_resource *buffer) = 0;
    virtual bool isProtected(struct ::wl_resource *buffer) { Q_UNUSED(buffer); return false; }

protected:
    WaylandCompositor *m_compositor = nullptr;
};

}

} // namespace Compositor

} // namespace Aurora

