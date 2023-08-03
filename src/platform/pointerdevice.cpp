// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "pointerdevice.h"

namespace Aurora {

namespace Platform {

PointerDevice::PointerDevice(QObject *parent)
    : InputDevice(parent)
{
}

InputDevice::DeviceType PointerDevice::deviceType()
{
    return DeviceType::Pointer;
}

} // namespace Platform

} // namespace Aurora
