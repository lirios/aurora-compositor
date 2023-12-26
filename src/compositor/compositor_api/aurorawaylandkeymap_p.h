// Copyright (C) 2017 The Qt Company Ltd.
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

#include <LiriAuroraCompositor/aurorawaylandkeymap.h>
#include <QtCore/private/qobject_p.h>

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandKeymapPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(WaylandKeymap)
public:
    WaylandKeymapPrivate(const QString &layout, const QString &variant, const QString &options,
                          const QString &model, const QString &rules);

    QString m_layout;
    QString m_variant;
    QString m_options;
    QString m_rules;
    QString m_model;
};

} // namespace Compositor

} // namespace Aurora

