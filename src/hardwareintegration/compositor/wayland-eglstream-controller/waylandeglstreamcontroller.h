/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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
