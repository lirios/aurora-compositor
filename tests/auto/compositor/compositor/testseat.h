// Copyright (C) 2016 LG Electronics, Inc., author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <LiriAuroraCompositor/WaylandSeat>
#include <QList>

class QInputEvent;
class QMouseEvent;

namespace Aurora {

namespace Compositor {

class TestSeat : public WaylandSeat
{
    Q_OBJECT
public:

    TestSeat(WaylandCompositor *compositor, WaylandSeat::CapabilityFlags caps);
    ~TestSeat() override;

    bool isOwner(QInputEvent *inputEvent) const override;

    QList<QMouseEvent *> createMouseEvents(int count);

    int queryCount() { return m_queryCount; }

private:
    mutable int m_queryCount;
    QList<QMouseEvent *> m_events;
};

} // namespace Compositor

} // namespace Aurora

