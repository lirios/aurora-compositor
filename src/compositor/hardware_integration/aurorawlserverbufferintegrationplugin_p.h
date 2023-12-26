// Copyright (C) 2019 The Qt Company Ltd.
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

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>
#include <QtCore/private/qglobal_p.h>

namespace Aurora {

namespace Compositor {

namespace Internal {

class ServerBufferIntegration;

#define AuroraServerBufferIntegrationFactoryInterface_iid "org.qt-project.Qt.Compositor.AuroraServerBufferIntegrationFactoryInterface.5.3"

class LIRIAURORACOMPOSITOR_EXPORT ServerBufferIntegrationPlugin : public QObject
{
    Q_OBJECT
public:
    explicit ServerBufferIntegrationPlugin(QObject *parent = nullptr);
    ~ServerBufferIntegrationPlugin() override;

    virtual ServerBufferIntegration *create(const QString &key, const QStringList &paramList) = 0;
};

}

} // namespace Compositor

} // namespace Aurora

