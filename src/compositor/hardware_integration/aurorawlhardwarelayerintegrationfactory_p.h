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
#include <QtCore/QStringList>
#include <QtCore/private/qglobal_p.h>

namespace Aurora {

namespace Compositor {

namespace Internal {

class HardwareLayerIntegration;

class LIRIAURORACOMPOSITOR_EXPORT HardwareLayerIntegrationFactory
{
public:
    static QStringList keys();
    static HardwareLayerIntegration *create(const QString &name, const QStringList &args);
};

}

} // namespace Compositor

} // namespace Aurora

