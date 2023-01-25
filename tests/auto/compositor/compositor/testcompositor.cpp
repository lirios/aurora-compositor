// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testcompositor.h"
#include "testseat.h"
#include "testkeyboardgrabber.h"

#include <wayland-server-core.h>

namespace Aurora {

namespace Compositor {

TestCompositor::TestCompositor(bool createInputDev)
    : shell(new WaylandWlShell(this))
    , m_createSeat(createInputDev)
{
    setSocketName("wayland-qt-test-0");
}

void TestCompositor::create()
{
    auto output = new WaylandOutput(this, nullptr);
    setDefaultOutput(output);

    WaylandCompositor::create();

    connect(this, &WaylandCompositor::surfaceCreated, this, &TestCompositor::onSurfaceCreated);
    connect(this, &WaylandCompositor::surfaceAboutToBeDestroyed, this, &TestCompositor::onSurfaceAboutToBeDestroyed);
}

void TestCompositor::flushClients()
{
    wl_display_flush_clients(display());
}

void TestCompositor::onSurfaceCreated(WaylandSurface *surface)
{
    surfaces << surface;
}

void TestCompositor::onSurfaceAboutToBeDestroyed(WaylandSurface *surface)
{
    surfaces.removeOne(surface);
}

WaylandSeat *TestCompositor::createSeat()
{
    if (m_createSeat)
        return new TestSeat(this, WaylandSeat::Pointer | WaylandSeat::Keyboard);
    else
        return WaylandCompositor::createSeat();
}

WaylandKeyboard *TestCompositor::createKeyboardDevice(WaylandSeat *seat) {
    return new TestKeyboardGrabber(seat);
}

} // namespace Compositor

} // namespace Aurora
