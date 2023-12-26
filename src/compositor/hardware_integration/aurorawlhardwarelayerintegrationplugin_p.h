// Copyright (C) 2018 The Qt Company Ltd.
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

class HardwareLayerIntegration;

#define QtWaylandHardwareLayerIntegrationFactoryInterface_iid "org.qt-project.Qt.Compositor.AuroraHardwareLayerIntegrationFactoryInterface.5.11"

class LIRIAURORACOMPOSITOR_EXPORT HardwareLayerIntegrationPlugin : public QObject
{
    Q_OBJECT
public:
    explicit HardwareLayerIntegrationPlugin(QObject *parent = nullptr);
    ~HardwareLayerIntegrationPlugin() override;

    virtual HardwareLayerIntegration *create(const QString &key, const QStringList &paramList) = 0;
};

}

} // namespace Compositor

} // namespace Aurora

