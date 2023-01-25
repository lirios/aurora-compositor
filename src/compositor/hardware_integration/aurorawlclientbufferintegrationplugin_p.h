// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDCLIENTBUFFERINTEGRATIONPLUGIN_H
#define AURORA_COMPOSITOR_WAYLANDCLIENTBUFFERINTEGRATIONPLUGIN_H

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

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

namespace Aurora {

namespace Compositor {

namespace Internal {

class ClientBufferIntegration;

#define AuroraClientBufferIntegrationFactoryInterface_iid "org.qt-project.Qt.WaylandCompositor.AuroraClientBufferIntegrationFactoryInterface.5.3"

class LIRIAURORACOMPOSITOR_EXPORT ClientBufferIntegrationPlugin : public QObject
{
    Q_OBJECT
public:
    explicit ClientBufferIntegrationPlugin(QObject *parent = nullptr);
    ~ClientBufferIntegrationPlugin() override;

    virtual ClientBufferIntegration *create(const QString &key, const QStringList &paramList) = 0;
};

}

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDCLIENTBUFFERINTEGRATIONPLUGIN_H
