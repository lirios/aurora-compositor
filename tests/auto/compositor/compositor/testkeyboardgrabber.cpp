// Copyright (C) 2016 LG Electronics, Inc., author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "testkeyboardgrabber.h"

namespace Aurora {

namespace Compositor {

TestKeyboardGrabber::TestKeyboardGrabber(WaylandSeat *seat)
    : WaylandKeyboard(seat)
{
}

void TestKeyboardGrabber::setFocus(WaylandSurface *surface)
{
    Q_EMIT focusedCalled();
    WaylandKeyboard::setFocus(surface);
}

void TestKeyboardGrabber::sendKeyPressEvent(uint code)
{
    Q_EMIT keyPressCalled();
    WaylandKeyboard::sendKeyPressEvent(code);
}

void TestKeyboardGrabber::sendKeyReleaseEvent(uint code)
{
    Q_EMIT keyReleaseCalled();
    WaylandKeyboard::sendKeyReleaseEvent(code);
}

void TestKeyboardGrabber::sendKeyModifiers(WaylandClient *client, uint32_t serial)
{
    Q_EMIT modifiersCalled();
    WaylandKeyboard::sendKeyModifiers(client, serial);
}

} // namespace Compositor

} // namespace Aurora
