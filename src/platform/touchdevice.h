// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <LiriAuroraPlatform/InputDevice>

namespace Aurora {

namespace Platform {

class LIRIAURORAPLATFORM_EXPORT TouchDevice : public InputDevice
{
    Q_OBJECT
public:
    explicit TouchDevice(QObject *parent = nullptr);

    DeviceType deviceType() override;
};

} // namespace Platform

} // namespace Aurora
