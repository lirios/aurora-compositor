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

#include <LiriAuroraCompositor/private/aurora-server-hardware-integration.h>

#include <LiriAuroraCompositor/WaylandCompositorExtension>

#include <QtCore/QString>
#include <QtCore/private/qglobal_p.h>

namespace Aurora {

namespace Compositor {

class WaylandCompositor;

namespace Internal {

class HardwareIntegration : public WaylandCompositorExtensionTemplate<HardwareIntegration>, public PrivateServer::qt_hardware_integration
{
public:
    HardwareIntegration(WaylandCompositor *compositor);

    void setClientBufferIntegrationName(const QString &name);
    void setServerBufferIntegrationName(const QString &name);

protected:
    void hardware_integration_bind_resource(Resource *resource) override;

private:
    QString m_client_buffer_integration_name;
    QString m_server_buffer_integration_name;
};

}

} // namespace Compositor

} // namespace Aurora
