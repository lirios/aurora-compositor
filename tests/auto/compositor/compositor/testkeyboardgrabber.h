// Copyright (C) 2016 LG Electronics, Inc., author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <LiriAuroraCompositor/WaylandKeyboard>

namespace Aurora {

namespace Compositor {

class TestKeyboardGrabber : public WaylandKeyboard
{
    Q_OBJECT
public:
    TestKeyboardGrabber(WaylandSeat *seat);

    void setFocus(WaylandSurface *surface) override;
    void sendKeyModifiers(WaylandClient *client, uint32_t serial) override;
    void sendKeyPressEvent(uint code) override;
    void sendKeyReleaseEvent(uint code) override;

signals:
    void focusedCalled();
    void keyPressCalled();
    void keyReleaseCalled();
    void modifiersCalled();
};

} // namespace Compositor

} // namespace Aurora
