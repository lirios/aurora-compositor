// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "touchdevice.h"

namespace Aurora {

namespace Platform {

TouchDevice::TouchDevice(QObject *parent)
    : InputDevice(parent)
{
}

InputDevice::DeviceType TouchDevice::deviceType()
{
    return DeviceType::Touch;
}

} // namespace Platform

} // namespace Aurora
