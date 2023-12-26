// Copyright (C) 2020 The Qt Company Ltd.
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

#include <LiriAuroraCompositor/WaylandQuickItem>
#include <QtCore/private/qglobal_p.h>

namespace Aurora {

namespace Compositor {

class WaylandQuickHardwareLayerPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickHardwareLayer : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_DECLARE_PRIVATE(WaylandQuickHardwareLayer)
    Q_PROPERTY(int stackingLevel READ stackingLevel WRITE setStackingLevel NOTIFY stackingLevelChanged)
    QML_NAMED_ELEMENT(WaylandHardwareLayer)
    QML_ADDED_IN_VERSION(1, 2)
public:
    explicit WaylandQuickHardwareLayer(QObject *parent = nullptr);
    ~WaylandQuickHardwareLayer() override;

    int stackingLevel() const;
    void setStackingLevel(int level);

    WaylandQuickItem *waylandItem() const;

    void classBegin() override;
    void componentComplete() override;

    void setSceneGraphPainting(bool);
    void initialize();

Q_SIGNALS:
    void stackingLevelChanged();
};

} // namespace Compositor

} // namespace Aurora

