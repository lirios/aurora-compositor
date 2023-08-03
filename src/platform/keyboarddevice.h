// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <LiriAuroraPlatform/InputDevice>

namespace Aurora {

namespace Platform {

class KeyboardDevicePrivate;

class LIRIAURORAPLATFORM_EXPORT KeyboardDevice : public InputDevice
{
    Q_OBJECT
    Q_PROPERTY(bool keyRepeatEnabled READ isKeyRepeatEnabled NOTIFY keyRepeatEnabledChanged)
    Q_PROPERTY(qint32 keyRepeatRate READ keyRepeatRate NOTIFY keyRepeatRateChanged)
    Q_PROPERTY(qint32 keyRepeatDelay READ keyRepeatDelay NOTIFY keyRepeatDelayChanged)
    Q_DECLARE_PRIVATE(KeyboardDevice)
public:
    enum class KeyState {
        Released,
        Pressed,
    };
    Q_ENUM(KeyState)

    class KeyEvent
    {
    public:
        int key;
        Qt::KeyboardModifiers modifiers;
        quint32 nativeScanCode;
        quint32 nativeVirtualKey;
        quint32 nativeModifiers;
        QString text;
        quint32 timestamp;
        bool autoRepeat;
        ushort repeatCount;
    };

    explicit KeyboardDevice(QObject *parent = nullptr);
    ~KeyboardDevice();

    DeviceType deviceType() override;

    bool isKeyRepeatEnabled() const;
    qint32 keyRepeatRate() const;
    qint32 keyRepeatDelay() const;

    Qt::KeyboardModifiers modifiers() const;

protected:
    QScopedPointer<KeyboardDevicePrivate> const d_ptr;

    void setKeyRepeatEnabled(bool enabled);
    void setKeyRepeatRate(qint32 value);
    void setKeyRepeatDelay(qint32 value);

    void handleKeymapChanged(int fd, quint32 size);
    void handleKeyChanged(quint32 key, const KeyState &keyState, quint32 time);
    void handleModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group);

Q_SIGNALS:
    void keyPressed(const KeyboardDevice::KeyEvent &event);
    void keyReleased(const KeyboardDevice::KeyEvent &event);
    void keyRepeatEnabledChanged(bool enabled);
    void keyRepeatRateChanged(qint32 rate);
    void keyRepeatDelayChanged(qint32 delay);
};

} // namespace Platform

} // namespace Aurora
