// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMap>
#include <QPointer>

#include <LiriAuroraPlatform/InputManager>

namespace Aurora {

namespace Platform {

class WaylandKeyboard;
class WaylandPointer;
class WaylandTouch;

typedef QMap<InputDevice::DeviceType, InputDevice *> InputDevicesMap;

class WaylandInputManager : public InputManager
{
    Q_OBJECT
public:
    WaylandInputManager(QObject *parent = nullptr);
    ~WaylandInputManager();

    QList<KeyboardDevice *> keyboardDevices() const override;
    QList<PointerDevice *> pointerDevices() const override;
    QList<TouchDevice *> touchDevices() const override;

    int deviceCount(InputDevice::DeviceType deviceType) const override;

private:
    WaylandKeyboard *m_keyboard = nullptr;
    WaylandPointer *m_pointer = nullptr;
    WaylandTouch *m_touch = nullptr;

private slots:
    void handleHasKeyboardChanged(bool hasKeyboard);
    void handleHasPointerChanged(bool hasPointer);
    void handleHasTouchChanged(bool hasTouch);
};

} // namespace Platform

} // namespace Aurora
