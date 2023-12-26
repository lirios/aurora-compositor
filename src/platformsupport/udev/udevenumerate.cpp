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

#include <LiriAuroraLogind/Logind>

#include "udev.h"
#include "udev_p.h"
#include "udevenumerate.h"
#include "udevenumerate_p.h"

namespace Aurora {

namespace PlatformSupport {

/*
 * UdevEnumeratePrivate
 */

UdevEnumeratePrivate::UdevEnumeratePrivate(UdevDevice::DeviceTypes t, Udev *u)
    : types(t)
    , udev(u)
{
    enumerate = udev_enumerate_new(UdevPrivate::get(u)->udev);
    if (!enumerate) {
        qCWarning(gLcUdev, "Unable to enumerate connected devices");
        return;
    }

    if (types.testFlag(UdevDevice::InputDevice_Mask))
        udev_enumerate_add_match_subsystem(enumerate, "input");

    if (types.testFlag(UdevDevice::VideoDevice_Mask)) {
        udev_enumerate_add_match_subsystem(enumerate, "drm");
        udev_enumerate_add_match_sysname(enumerate, "card[0-9]*");
    }

    if (types.testFlag(UdevDevice::KeyboardDevice)) {
        udev_enumerate_add_match_property(enumerate, "ID_INPUT_KEYBOARD", "1");
        udev_enumerate_add_match_property(enumerate, "ID_INPUT_KEY", "1");
    }

    if (types.testFlag(UdevDevice::MouseDevice))
        udev_enumerate_add_match_property(enumerate, "ID_INPUT_MOUSE", "1");

    if (types.testFlag(UdevDevice::TouchpadDevice))
        udev_enumerate_add_match_property(enumerate, "ID_INPUT_TOUCHPAD", "1");

    if (types.testFlag(UdevDevice::TouchscreenDevice))
        udev_enumerate_add_match_property(enumerate, "ID_INPUT_TOUCHSCREEN", "1");

    if (types.testFlag(UdevDevice::TabletDevice))
        udev_enumerate_add_match_property(enumerate, "ID_INPUT_TABLET", "1");

    if (types.testFlag(UdevDevice::JoystickDevice))
        udev_enumerate_add_match_property(enumerate, "ID_INPUT_JOYSTICK", "1");
}

UdevEnumeratePrivate::~UdevEnumeratePrivate()
{
    if (enumerate)
        udev_enumerate_unref(enumerate);
}

/*
 * UdevEnumerate
 */

UdevEnumerate::UdevEnumerate(UdevDevice::DeviceTypes types, Udev *udev)
    : d_ptr(new UdevEnumeratePrivate(types, udev))
{
    qRegisterMetaType<UdevDevice>();
}

UdevEnumerate::~UdevEnumerate()
{
    delete d_ptr;
}

QList<UdevDevice *> UdevEnumerate::scan() const
{
    Q_D(const UdevEnumerate);

    QList<UdevDevice *> list;

    if (!d->enumerate)
        return list;

    if (udev_enumerate_scan_devices(d->enumerate) != 0) {
        qCWarning(gLcUdev, "Unable to enumerate connected devices");
        return list;
    }

    udev_device *drmDevice = nullptr;
    udev_device *drmPrimaryDevice = nullptr;

    udev_list_entry *entry;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(d->enumerate)) {
        const char *syspath = udev_list_entry_get_name(entry);
        udev_device *dev = udev_device_new_from_syspath(UdevPrivate::get(d->udev)->udev, syspath);
        if (!dev)
            continue;

        // Must be on the same seat
        QString seat = QString::fromUtf8(udev_device_get_property_value(dev, "ID_SEAT"));
        if (seat.isEmpty())
            seat = QStringLiteral("seat0");
        if (seat != Logind::instance()->seat()) {
            udev_device_unref(dev);
            continue;
        }

        QString node = QString::fromUtf8(udev_device_get_devnode(dev));

        if (d->types.testFlag(UdevDevice::InputDevice_Mask) && node.startsWith(QLatin1String("/dev/input/event"))) {
            UdevDevice *device = new UdevDevice;
            device->initialize(dev);
            list.append(device);
        }

        if (d->types.testFlag(UdevDevice::VideoDevice_Mask) && node.startsWith(QLatin1String("/dev/dri/card"))) {
            // We can have more than one DRM device on our seat, so the filter
            // might want us to pick up only the primary video device
            // In any case we'll be adding just one DRM device to the list
            if (d->types.testFlag(UdevDevice::PrimaryVideoDevice)) {
                udev_device *pci =
                        udev_device_get_parent_with_subsystem_devtype(dev, "pci", nullptr);
                if (pci) {
                    if (qstrcmp(udev_device_get_sysattr_value(pci, "boot_vga"), "1") == 0)
                        drmPrimaryDevice = dev;
                }
            }
            if (!drmPrimaryDevice) {
                if (drmDevice)
                    udev_device_unref(drmDevice);
                drmDevice = dev;
            }
        }
    }

    // Add any DRM device previously enumerated
    if (drmPrimaryDevice) {
        UdevDevice *device = new UdevDevice;
        device->initialize(drmPrimaryDevice);
        list.append(device);
    } else if (drmDevice) {
        UdevDevice *device = new UdevDevice;
        device->initialize(drmDevice);
        list.append(device);
    }

    return list;
}

} // namespace PlatformSupport

} // namespace Aurora
