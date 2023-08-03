// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>

#include <LiriAuroraPlatform/liriauroraplatformglobal.h>

namespace Aurora {

namespace Platform {

class LIRIAURORAPLATFORM_EXPORT InputDevice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(DeviceType deviceType READ deviceType CONSTANT)
public:
    enum class DeviceType {
        Unknown,
        Pointer,
        Keyboard,
        Touch,
        Tablet
    };
    Q_ENUM(DeviceType)

    explicit InputDevice(QObject *parent = nullptr);

    virtual QString seatName() const = 0;

    virtual DeviceType deviceType() = 0;
};

} // namespace Platform

} // namespace Aurora
