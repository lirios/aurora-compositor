/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPLv3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef QTUDEVDEVICE_H
#define QTUDEVDEVICE_H

#include <QtCore/QObject>

#include <LiriAuroraUdev/liriauroraudevglobal.h>

struct udev_device;

namespace Aurora {

namespace PlatformSupport {

class Udev;
class UdevEnumerate;
class UdevDevicePrivate;
class UdevMonitorPrivate;

class LIRIAURORAUDEV_EXPORT UdevDevice
{
    Q_DECLARE_PRIVATE(UdevDevice)
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
        InputDevice_Mask = KeyboardDevice | MouseDevice | TouchpadDevice | TouchscreenDevice | TabletDevice | JoystickDevice,
        VideoDevice_Mask = GenericVideoDevice
    };
    Q_DECLARE_FLAGS(DeviceTypes, DeviceType)

    explicit UdevDevice();
    ~UdevDevice();

    bool isValid() const;

    DeviceTypes type() const;

    QString subsystem() const;
    QString devType() const;
    QString name() const;
    QString driver() const;

    QString deviceNode() const;
    QStringList alternateDeviceSymlinks() const;

    QString sysfsPath() const;
    int sysfsNumber() const;

    QString property(const QString &name) const;
    bool hasProperty(const QString &name) const;

    QStringList deviceProperties() const;
    QStringList sysfsProperties() const;

    UdevDevice *parent() const;

private:
    void initialize(udev_device *dev);

    UdevDevicePrivate *const d_ptr;

    friend class Udev;
    friend class UdevEnumerate;
    friend class UdevMonitorPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(UdevDevice::DeviceTypes)

QDebug operator<<(QDebug, const UdevDevice &);

} // namespace PlatformSupport

} // namespace Aurora

Q_DECLARE_METATYPE(Aurora::PlatformSupport::UdevDevice)

#endif // QTUDEVDEVICE_H
