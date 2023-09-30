// SPDX-FileCopyrightText: 2018-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QString>

struct udev;
struct udev_device;
struct udev_monitor;
struct udev_enumerate;

namespace Aurora {

namespace Platform {

class UdevDevice : public QObject
{
    Q_OBJECT
public:
    enum DeviceType {
        UnknownDevice = 0x00,
        KeyboardDevice = 0x01,
        MouseDevice = 0x02,
        TouchpadDevice = 0x04,
        TouchscreenDevice = 0x08,
        TabletDevice = 0x10,
        JoystickDevice = 0x20,
        GenericVideoDevice = 0x40,
        PrimaryVideoDevice = 0x80,
        InputDevice_Mask = KeyboardDevice | MouseDevice | TouchpadDevice | TouchscreenDevice
                | TabletDevice | JoystickDevice,
        VideoDevice_Mask = GenericVideoDevice
    };
    Q_DECLARE_FLAGS(DeviceTypes, DeviceType)

    ~UdevDevice();

    bool isValid() const;

    DeviceTypes type() const;

    QString subsystem() const;
    QString devType() const;
    QString name() const;
    QString driver() const;

    QString deviceNode() const;
    dev_t deviceId() const;
    QStringList alternateDeviceSymlinks() const;

    QString sysfsPath() const;
    int sysfsNumber() const;

    QString seat() const;

    QString property(const QString &name) const;
    bool hasProperty(const QString &name) const;

    QStringList deviceProperties() const;
    QStringList sysfsProperties() const;

    UdevDevice *parent() const;

    operator struct udev_device *() const
    {
        return m_device;
    }
    operator struct udev_device *()
    {
        return m_device;
    }

private:
    explicit UdevDevice(struct udev_device *device, QObject *parent = nullptr);

    struct udev_device *m_device = nullptr;

    friend class Udev;
    friend class UdevEnumerate;
    friend class UdevMonitor;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(UdevDevice::DeviceTypes)

QDebug operator<<(QDebug, const UdevDevice &);

//

class Udev : public QObject
{
    Q_OBJECT
public:
    explicit Udev(QObject *parent = nullptr);
    ~Udev();

    bool isValid() const;

    UdevDevice *deviceFromFileName(const QString &fileName);
    UdevDevice *deviceFromSubSystemAndName(const QString &subSystem, const QString &name);
    UdevDevice *deviceFromSysfsPath(const QString &sysfsPath);

    operator struct udev *() const
    {
        return m_udev;
    }
    operator struct udev *()
    {
        return m_udev;
    }

private:
    struct udev *m_udev = nullptr;
};

//

class UdevEnumerate : public QObject
{
    Q_OBJECT
public:
    explicit UdevEnumerate(UdevDevice::DeviceTypes types, Udev *udev, QObject *parent = nullptr);
    ~UdevEnumerate();

    QList<UdevDevice *> scan() const;

    operator struct udev_enumerate *() const
    {
        return m_enumerate;
    }
    operator struct udev_enumerate *()
    {
        return m_enumerate;
    }

private:
    UdevDevice::DeviceTypes m_types = UdevDevice::UnknownDevice;
    Udev *m_udev = nullptr;
    struct udev_enumerate *m_enumerate = nullptr;
};

//

class UdevMonitor : public QObject
{
    Q_OBJECT
public:
    explicit UdevMonitor(Udev *udev, QObject *parent = nullptr);
    ~UdevMonitor();

    bool isValid() const;

    int fd() const;

    void filterSubSystemDevType(const QString &subSystem, const QString &devType = QString());
    void filterTag(const QString &tag);

    operator struct udev_monitor *() const
    {
        return m_monitor;
    }
    operator struct udev_monitor *()
    {
        return m_monitor;
    }

Q_SIGNALS:
    void deviceAdded(Aurora::Platform::UdevDevice *device);
    void deviceRemoved(Aurora::Platform::UdevDevice *device);
    void deviceChanged(Aurora::Platform::UdevDevice *device);
    void deviceOnlined(Aurora::Platform::UdevDevice *device);
    void deviceOfflined(Aurora::Platform::UdevDevice *device);

private:
    Udev *m_udev = nullptr;
    struct udev_monitor *m_monitor = nullptr;

private Q_SLOTS:
    void handleUdevEvent();
};

} // namespace Platform

} // namespace Aurora

Q_DECLARE_METATYPE(Aurora::Platform::UdevDevice)
