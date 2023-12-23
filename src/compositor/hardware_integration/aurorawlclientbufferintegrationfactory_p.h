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
#include <QtCore/QStringList>

namespace Aurora {

namespace Compositor {

namespace Internal {

class ClientBufferIntegration;

class LIRIAURORACOMPOSITOR_EXPORT ClientBufferIntegrationFactory
{
public:
    static QStringList keys();
    static ClientBufferIntegration *create(const QString &name, const QStringList &args);
};

}

} // namespace Compositor

} // namespace Aurora

