/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef BRCMEGLINTEGRATION_H
#define BRCMEGLINTEGRATION_H

#include <LiriAuroraCompositor/private/aurorawlclientbufferintegration_p.h>
#include "aurora-server-brcm.h"

#include <QtCore/QScopedPointer>

#include <private/aurorawlclientbuffer_p.h>

namespace Aurora {

namespace Compositor {

class BrcmEglIntegrationPrivate;

class BrcmEglIntegration : public Internal::ClientBufferIntegration, public PrivateServer::qt_brcm
{
    Q_DECLARE_PRIVATE(BrcmEglIntegration)
public:
    BrcmEglIntegration();

    void initializeHardware(struct ::wl_display *display) override;
    Internal::ClientBuffer *createBufferFor(wl_resource *buffer) override;

protected:
    void brcm_bind_resource(Resource *resource) override;
    void brcm_create_buffer(Resource *resource, uint32_t id, int32_t width, int32_t height, wl_array *data) override;

private:
    Q_DISABLE_COPY(BrcmEglIntegration)
    QScopedPointer<BrcmEglIntegrationPrivate> d_ptr;
};

class BrcmEglClientBuffer : public Internal::ClientBuffer
{
public:
    BrcmEglClientBuffer(BrcmEglIntegration *integration, wl_resource *buffer);

    WaylandBufferRef::BufferFormatEgl bufferFormatEgl() const override;
    QSize size() const override;
    WaylandSurface::Origin origin() const override;
    QOpenGLTexture *toOpenGlTexture(int plane) override;
private:
    BrcmEglIntegration *m_integration = nullptr;
    QOpenGLTexture *m_texture = nullptr;
};


} // namespace Compositor

} // namespace Aurora

#endif // BRCMEGLINTEGRATION_H

