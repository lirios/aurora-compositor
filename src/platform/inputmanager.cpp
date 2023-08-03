// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inputmanager.h"

namespace Aurora {

namespace Platform {

InputManager::InputManager(QObject *parent)
    : QObject(parent)
{
}

QList<KeyboardDevice *> InputManager::keyboardDevices() const
{
    return QList<KeyboardDevice *>();
}

QList<PointerDevice *> InputManager::pointerDevices() const
{
    return QList<PointerDevice *>();
}

QList<TouchDevice *> InputManager::touchDevices() const
{
    return QList<TouchDevice *>();
}

int InputManager::deviceCount(InputDevice::DeviceType deviceType) const
{
    Q_UNUSED(deviceType)
    return 0;
}

} // namespace Platform

} // namespace Aurora
