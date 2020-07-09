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

#include "qwlhwintegration_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>

QT_BEGIN_NAMESPACE

namespace QtWayland {

HardwareIntegration::HardwareIntegration(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<HardwareIntegration>(compositor)
    , qt_hardware_integration(compositor->display(), 1)
{
}

void HardwareIntegration::setClientBufferIntegrationName(const QString &name)
{
    m_client_buffer_integration_name = name;
}
void HardwareIntegration::setServerBufferIntegrationName(const QString &name)
{
    m_server_buffer_integration_name = name;
}

void HardwareIntegration::hardware_integration_bind_resource(Resource *resource)
{
    if (!m_client_buffer_integration_name.isEmpty())
        send_client_backend(resource->handle, m_client_buffer_integration_name);
    if (!m_server_buffer_integration_name.isEmpty())
        send_server_backend(resource->handle, m_server_buffer_integration_name);
}

}

QT_END_NAMESPACE
