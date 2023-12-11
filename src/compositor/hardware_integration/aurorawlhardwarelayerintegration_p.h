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

#include <QObject>
#include <private/qglobal_p.h>

namespace Aurora {

namespace Compositor {

class QPoint;

class WaylandQuickHardwareLayer;

namespace Internal {

class LIRIAURORACOMPOSITOR_EXPORT HardwareLayerIntegration : public QObject
{
    Q_OBJECT
public:
    HardwareLayerIntegration(QObject *parent = nullptr)
        : QObject(parent)
    {}
    ~HardwareLayerIntegration() override {}
    virtual void add(WaylandQuickHardwareLayer *) {}
    virtual void remove(WaylandQuickHardwareLayer *) {}
};

} // namespace Internal

} // namespace Compositor

} // namespace Aurora

