// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QtQml/QQmlListProperty>
#include <QtQml/QQmlParserStatus>
#include <LiriAuroraCompositor/WaylandXdgOutputV1>

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickXdgOutputV1
        : public WaylandXdgOutputV1
        , public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
public:
    explicit WaylandQuickXdgOutputV1();

protected:
    void classBegin() override {}
    void componentComplete() override;
};

} // namespace Compositor

} // namespace Aurora

