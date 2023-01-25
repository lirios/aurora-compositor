// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDQUICKSHELLINTEGRATION_H
#define AURORA_COMPOSITOR_WAYLANDQUICKSHELLINTEGRATION_H

#include <QtCore/QObject>
#include <LiriAuroraCompositor/liriauroracompositorglobal.h>

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickShellIntegration : public QObject
{
    Q_OBJECT
public:
    WaylandQuickShellIntegration(QObject *parent = nullptr);
    ~WaylandQuickShellIntegration() override;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDQUICKSHELLINTEGRATION_H
