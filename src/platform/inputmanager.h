// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>

#include <LiriAuroraPlatform/InputDevice>

namespace Aurora {

namespace Platform {

class KeyboardDevice;
class PointerDevice;
class TouchDevice;

class LIRIAURORAPLATFORM_EXPORT InputManager : public QObject
{
    Q_OBJECT
public:
    explicit InputManager(QObject *parent = nullptr);

    virtual QList<KeyboardDevice *> keyboardDevices() const;
    virtual QList<PointerDevice *> pointerDevices() const;
    virtual QList<TouchDevice *> touchDevices() const;

    virtual int deviceCount(InputDevice::DeviceType deviceType) const;

Q_SIGNALS:
    void deviceAdded(InputDevice *inputDevice);
    void deviceRemoved(InputDevice *inputDevice);
    void keyboardAdded(KeyboardDevice *keyboardDevice);
    void keyboardRemoved(KeyboardDevice *keyboardDevice);
    void pointerAdded(PointerDevice *pointerDevice);
    void pointerRemoved(PointerDevice *pointerDevice);
    void touchAdded(TouchDevice *touchDevice);
    void touchRemoved(TouchDevice *touchDevice);
};

} // namespace Platform

} // namespace Aurora
