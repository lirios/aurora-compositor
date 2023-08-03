// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <LiriAuroraPlatform/InputDevice>

namespace Aurora {

namespace Platform {

class LIRIAURORAPLATFORM_EXPORT PointerDevice : public InputDevice
{
    Q_OBJECT
public:
    explicit PointerDevice(QObject *parent = nullptr);

    DeviceType deviceType() override;

Q_SIGNALS:
    void motion(const QPointF &absPosition);
    void buttonPressed(Qt::MouseButton button);
    void buttonReleased(Qt::MouseButton button);
};

} // namespace Platform

} // namespace Aurora
