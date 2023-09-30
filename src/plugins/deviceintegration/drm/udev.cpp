// SPDX-FileCopyrightText: 2018-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QSocketNotifier>

#include <qplatformdefs.h>

#include "drmloggingcategories.h"
#include "udev.h"

extern "C" {
#include <libudev.h>
}

namespace Aurora {

namespace Platform {

static inline QStringList listFromEntries(udev_list_entry *l)
{
    QStringList list;
    struct udev_list_entry *entry;

    udev_list_entry_foreach(entry, l)
    {
        list.append(QString::fromUtf8(udev_list_entry_get_name(entry)));
    }

    return list;
}

/*
 * Udev
 */

Udev::Udev(QObject *parent)
    : QObject(parent)
    , m_udev(udev_new())
{
    if (!m_udev)
        qCWarning(gLcDrm, "Unable to get udev library context: no devices can be detected");
}

Udev::~Udev()
{
    if (m_udev)
        udev_unref(m_udev);
}

bool Udev::isValid() const
{
    return m_udev != nullptr;
}

UdevDevice *Udev::deviceFromFileName(const QString &fileName)
{
    if (!isValid())
        return nullptr;

    QT_STATBUF sb;

    if (QT_STAT(qPrintable(fileName), &sb) != 0)
        return nullptr;

    struct udev_device *dev = nullptr;

    if (S_ISBLK(sb.st_mode))
        dev = udev_device_new_from_devnum(m_udev, 'b', sb.st_rdev);
    else if (S_ISCHR(sb.st_mode))
        dev = udev_device_new_from_devnum(m_udev, 'c', sb.st_rdev);

    if (!dev)
        return nullptr;

    return new UdevDevice(dev, this);
}

UdevDevice *Udev::deviceFromSubSystemAndName(const QString &subSystem, const QString &name)
{
    if (!isValid())
        return nullptr;

    auto *dev =
            udev_device_new_from_subsystem_sysname(m_udev, qPrintable(subSystem), qPrintable(name));
    return new UdevDevice(dev, this);
}

UdevDevice *Udev::deviceFromSysfsPath(const QString &sysfsPath)
{
    if (!isValid())
        return nullptr;

    auto *dev = udev_device_new_from_syspath(m_udev, qPrintable(sysfsPath));
    return new UdevDevice(dev, this);
}

/*
 * UdevDevice
 */

UdevDevice::UdevDevice(struct udev_device *device, QObject *parent)
    : QObject(parent)
    , m_device(device)
{
}

UdevDevice::~UdevDevice()
{
    if (m_device)
        udev_device_unref(m_device);
}

bool UdevDevice::isValid() const
{
    return m_device != nullptr;
}

UdevDevice::DeviceTypes UdevDevice::type() const
{
    DeviceTypes result;

    if (!m_device)
        return result;

    if (qstrcmp(udev_device_get_property_value(m_device, "ID_INPUT_KEYBOARD"), "1") == 0)
        result |= KeyboardDevice;
    if (qstrcmp(udev_device_get_property_value(m_device, "ID_INPUT_KEY"), "1") == 0)
        result |= KeyboardDevice;

    if (qstrcmp(udev_device_get_property_value(m_device, "ID_INPUT_MOUSE"), "1") == 0)
        result |= MouseDevice;

    if (qstrcmp(udev_device_get_property_value(m_device, "ID_INPUT_TOUCHPAD"), "1") == 0)
        result |= TouchpadDevice;

    if (qstrcmp(udev_device_get_property_value(m_device, "ID_INPUT_TOUCHSCREEN"), "1") == 0)
        result |= TouchscreenDevice;

    if (qstrcmp(udev_device_get_property_value(m_device, "ID_INPUT_TABLET"), "1") == 0)
        result |= TabletDevice;

    if (qstrcmp(udev_device_get_property_value(m_device, "ID_INPUT_JOYSTICK"), "1") == 0)
        result |= JoystickDevice;

    if (qstrcmp(udev_device_get_subsystem(m_device), "drm") == 0) {
        bool isSet = false;

        auto *pci = udev_device_get_parent_with_subsystem_devtype(m_device, "pci", nullptr);
        if (pci) {
            if (qstrcmp(udev_device_get_sysattr_value(pci, "boot_vga"), "1") == 0) {
                result |= PrimaryVideoDevice;
                isSet = true;
            }
        }

        if (!isSet)
            result |= GenericVideoDevice;
    }

    return result;
}

QString UdevDevice::subsystem() const
{
    if (!m_device)
        return QString();
    return QString::fromUtf8(udev_device_get_subsystem(m_device));
}

QString UdevDevice::devType() const
{
    if (!m_device)
        return QString();
    return QString::fromUtf8(udev_device_get_devtype(m_device));
}

QString UdevDevice::name() const
{
    if (!m_device)
        return QString();
    return QString::fromUtf8(udev_device_get_sysname(m_device));
}

QString UdevDevice::driver() const
{
    if (!m_device)
        return QString();
    return QString::fromUtf8(udev_device_get_driver(m_device));
}

QString UdevDevice::deviceNode() const
{
    if (!m_device)
        return QString();
    return QString::fromUtf8(udev_device_get_devnode(m_device));
}

dev_t UdevDevice::deviceId() const
{
    return udev_device_get_devnum(m_device);
}

QStringList UdevDevice::alternateDeviceSymlinks() const
{
    if (!m_device)
        return QStringList();
    return listFromEntries(udev_device_get_devlinks_list_entry(m_device));
}

QString UdevDevice::sysfsPath() const
{
    if (!m_device)
        return QString();
    return QString::fromUtf8(udev_device_get_syspath(m_device));
}

int UdevDevice::sysfsNumber() const
{
    if (!m_device)
        return -1;
    return QByteArray(udev_device_get_sysnum(m_device)).toInt();
}

QString UdevDevice::seat() const
{
    QString seat = QString::fromUtf8(udev_device_get_property_value(m_device, "ID_SEAT"));
    if (seat.isEmpty())
        seat = QStringLiteral("seat0");
    return seat;
}

QString UdevDevice::property(const QString &name) const
{
    if (!m_device)
        return QString();
    return QString::fromLatin1(
            udev_device_get_property_value(m_device, name.toLatin1().constData()));
}

bool UdevDevice::hasProperty(const QString &name) const
{
    if (!m_device)
        return false;
    return udev_device_get_property_value(m_device, name.toLatin1().constData()) != nullptr;
}

QStringList UdevDevice::deviceProperties() const
{
    if (!m_device)
        return QStringList();
    return listFromEntries(udev_device_get_properties_list_entry(m_device));
}

QStringList UdevDevice::sysfsProperties() const
{
    if (!m_device)
        return QStringList();
    return listFromEntries(udev_device_get_sysattr_list_entry(m_device));
}

UdevDevice *UdevDevice::parent() const
{
    if (!m_device)
        return nullptr;

    auto *p = udev_device_get_parent(m_device);
    if (p)
        return new UdevDevice(p, parent());
    return nullptr;
}

QDebug operator<<(QDebug dbg, const UdevDevice &device)
{
    QDebugStateSaver saver(dbg);
    if (device.isValid())
        dbg.nospace() << "UdevDevice(" << device.deviceNode() << ')';
    else
        dbg.nospace() << "Invalid UdevDevice)";
    return dbg;
}

/*
 * UdevEnumerate
 */

UdevEnumerate::UdevEnumerate(UdevDevice::DeviceTypes types, Udev *udev, QObject *parent)
    : QObject(parent)
    , m_types(types)
    , m_udev(udev)
{
    qRegisterMetaType<UdevDevice>();

    m_enumerate = udev_enumerate_new(*udev);
    if (!m_enumerate) {
        qCWarning(gLcDrm, "Unable to enumerate connected devices");
        return;
    }

    if (types.testFlag(UdevDevice::InputDevice_Mask))
        udev_enumerate_add_match_subsystem(m_enumerate, "input");

    if (types.testFlag(UdevDevice::VideoDevice_Mask)) {
        udev_enumerate_add_match_subsystem(m_enumerate, "drm");
        udev_enumerate_add_match_sysname(m_enumerate, "card[0-9]*");
    }

    if (types.testFlag(UdevDevice::KeyboardDevice)) {
        udev_enumerate_add_match_property(m_enumerate, "ID_INPUT_KEYBOARD", "1");
        udev_enumerate_add_match_property(m_enumerate, "ID_INPUT_KEY", "1");
    }

    if (types.testFlag(UdevDevice::MouseDevice))
        udev_enumerate_add_match_property(m_enumerate, "ID_INPUT_MOUSE", "1");

    if (types.testFlag(UdevDevice::TouchpadDevice))
        udev_enumerate_add_match_property(m_enumerate, "ID_INPUT_TOUCHPAD", "1");

    if (types.testFlag(UdevDevice::TouchscreenDevice))
        udev_enumerate_add_match_property(m_enumerate, "ID_INPUT_TOUCHSCREEN", "1");

    if (types.testFlag(UdevDevice::TabletDevice))
        udev_enumerate_add_match_property(m_enumerate, "ID_INPUT_TABLET", "1");

    if (types.testFlag(UdevDevice::JoystickDevice))
        udev_enumerate_add_match_property(m_enumerate, "ID_INPUT_JOYSTICK", "1");
}

UdevEnumerate::~UdevEnumerate()
{
    if (m_enumerate)
        udev_enumerate_unref(m_enumerate);
}

QList<UdevDevice *> UdevEnumerate::scan() const
{
    QList<UdevDevice *> list;

    if (!m_enumerate)
        return list;

    if (udev_enumerate_scan_devices(m_enumerate) != 0) {
        qCWarning(gLcDrm, "Unable to enumerate connected devices");
        return list;
    }

    udev_device *drmDevice = nullptr;
    udev_device *drmPrimaryDevice = nullptr;

    udev_list_entry *entry;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(m_enumerate))
    {
        const char *syspath = udev_list_entry_get_name(entry);
        udev_device *dev = udev_device_new_from_syspath(*m_udev, syspath);
        if (!dev)
            continue;

        QString node = QString::fromUtf8(udev_device_get_devnode(dev));

        if (m_types.testFlag(UdevDevice::InputDevice_Mask)
            && node.startsWith(QLatin1String("/dev/input/event")))
            list.append(new UdevDevice(dev, m_udev));

        if (m_types.testFlag(UdevDevice::VideoDevice_Mask)
            && node.startsWith(QLatin1String("/dev/dri/card"))) {
            // We can have more than one DRM device on our seat, so the filter
            // might want us to pick up only the primary video device
            // In any case we'll be adding just one DRM device to the list
            if (m_types.testFlag(UdevDevice::PrimaryVideoDevice)) {
                auto *pci = udev_device_get_parent_with_subsystem_devtype(dev, "pci", nullptr);
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
    if (drmPrimaryDevice)
        list.append(new UdevDevice(drmPrimaryDevice, m_udev));
    else if (drmDevice)
        list.append(new UdevDevice(drmDevice, m_udev));

    return list;
}

/*
 * UdevMonitor
 */

UdevMonitor::UdevMonitor(Udev *udev, QObject *parent)
    : QObject(parent)
    , m_udev(udev)
{
    qRegisterMetaType<UdevDevice>();

    m_monitor = udev_monitor_new_from_netlink(*m_udev, "udev");
    if (!m_monitor) {
        qCWarning(gLcDrm, "Unable to create an udev monitor: no devices can be detected");
        return;
    }

    udev_monitor_enable_receiving(m_monitor);

    int fd = udev_monitor_get_fd(m_monitor);
    auto *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, SIGNAL(activated(int)), this, SLOT(handleUdevEvent()));
}

UdevMonitor::~UdevMonitor()
{
    if (m_monitor)
        udev_monitor_unref(m_monitor);
}

bool UdevMonitor::isValid() const
{
    return m_monitor != nullptr;
}

int UdevMonitor::fd() const
{
    if (m_monitor)
        return udev_monitor_get_fd(m_monitor);
    return -1;
}

void UdevMonitor::filterSubSystemDevType(const QString &subSystem, const QString &devType)
{
    if (!isValid())
        return;

    udev_monitor_filter_add_match_subsystem_devtype(m_monitor, qPrintable(subSystem),
                                                    qPrintable(devType));
}

void UdevMonitor::filterTag(const QString &tag)
{
    if (!isValid())
        return;

    udev_monitor_filter_add_match_tag(m_monitor, qPrintable(tag));
}

void UdevMonitor::handleUdevEvent()
{
    auto *dev = udev_monitor_receive_device(m_monitor);
    if (!dev)
        return;

    const char *action = udev_device_get_action(dev);
    if (!action) {
        udev_device_unref(dev);
        return;
    }

    auto *device = new UdevDevice(dev, m_udev);

    if (qstrcmp(action, "add") == 0)
        Q_EMIT deviceAdded(device);
    else if (qstrcmp(action, "remove") == 0)
        Q_EMIT deviceRemoved(device);
    else if (qstrcmp(action, "change") == 0)
        Q_EMIT deviceChanged(device);
    else if (qstrcmp(action, "online") == 0)
        Q_EMIT deviceOnlined(device);
    else if (qstrcmp(action, "offline") == 0)
        Q_EMIT deviceOfflined(device);
}

} // namespace Platform

} // namespace Aurora
