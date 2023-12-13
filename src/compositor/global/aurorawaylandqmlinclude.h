// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDQMLINCLUDE_H
#define AURORA_COMPOSITOR_WAYLANDQMLINCLUDE_H

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

#include <QtCore/qglobal.h>
#include <LiriAuroraGlobalPrivate/aurorafeatures.h>

#if LIRI_FEATURE_aurora_compositor_quick
#include <QtQml/qqml.h>
#else
#define QML_NAMED_ELEMENT(x)
#define QML_UNCREATABLE(x)
#define QML_ADDED_IN_VERSION(x, y)
#endif

#endif // AURORA_COMPOSITOR_WAYLANDQMLINCLUDE_H
