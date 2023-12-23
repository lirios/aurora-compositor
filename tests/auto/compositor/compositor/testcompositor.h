// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandSurface>
#include <LiriAuroraCompositor/WaylandWlShell>

namespace Aurora {

namespace Compositor {

class TestCompositor : public WaylandCompositor
{
    Q_OBJECT
public:
    TestCompositor(bool createInputDev = false);
    void create() override;
    void flushClients();

public slots:
    void onSurfaceCreated(Aurora::Compositor::WaylandSurface *surface);
    void onSurfaceAboutToBeDestroyed(Aurora::Compositor::WaylandSurface *surface);

protected:
    WaylandSeat *createSeat() override;
    WaylandKeyboard *createKeyboardDevice(WaylandSeat *seat) override;

public:
    QList<WaylandSurface *> surfaces;
    WaylandWlShell* shell;
    bool m_createSeat;
};

} // namespace Compositor

} // namespace Aurora

