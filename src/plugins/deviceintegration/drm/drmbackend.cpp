// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>
#include <QFileInfo>
#include <QSocketNotifier>

#include "drmbackend.h"
#include "drmdevice.h"
#include "drmloggingcategories.h"
#include "filedescriptor.h"
#include "udev.h"

#include <sys/stat.h>

#include <xf86drmMode.h>

#ifndef EGL_EXT_platform_base
typedef EGLDisplay(EGLAPIENTRYP PFNEGLGETPLATFORMDISPLAYEXTPROC)(EGLenum platform,
                                                                 void *native_display,
                                                                 const EGLint *attrib_list);
#endif

#ifndef EGL_PLATFORM_GBM_KHR
#  define EGL_PLATFORM_GBM_KHR 0x31D7
#endif

using namespace Qt::StringLiterals;

namespace Aurora {

namespace Platform {

Q_GLOBAL_STATIC(DrmBackend, gDrmBackend)

static QStringList splitPathList(const QString &input, const QChar delimiter)
{
    QStringList ret;
    QString tmp;
    for (int i = 0; i < input.size(); i++) {
        if (input[i] == delimiter) {
            if (i > 0 && input[i - 1] == QLatin1Char('\\')) {
                tmp[tmp.size() - 1] = delimiter;
            } else if (!tmp.isEmpty()) {
                ret.append(tmp);
                tmp = QString();
            }
        } else {
            tmp.append(input[i]);
        }
    }
    if (!tmp.isEmpty())
        ret.append(tmp);
    return ret;
}

DrmBackend::DrmBackend(QObject *parent)
    : QObject(parent)
    , m_session(Session::create(this))
    , m_udev(new Udev(this))
    , m_udevMonitor(new UdevMonitor(m_udev, this))
    , m_explicitDevices(splitPathList(qEnvironmentVariable("AURORA_DRM_DEVICES"), QLatin1Char(':')))
{
    if (m_session)
        qCInfo(gLcDrm, "Session support: %s", qPrintable(m_session->name()));
    else
        qCFatal(gLcDrm, "Session support not available, aborting...");

    connect(m_udevMonitor, &UdevMonitor::deviceAdded, this, &DrmBackend::handleDeviceAdded);
    connect(m_udevMonitor, &UdevMonitor::deviceRemoved, this, &DrmBackend::handleDeviceRemoved);
    connect(m_udevMonitor, &UdevMonitor::deviceChanged, this, &DrmBackend::handleDeviceChanged);
}

DrmBackend::~DrmBackend()
{
    if (m_udevMonitor) {
        m_udevMonitor->deleteLater();
        m_udevMonitor = nullptr;
    }

    if (m_udev) {
        m_udev->deleteLater();
        m_udev = nullptr;
    }
}

Session *DrmBackend::session() const
{
    return m_session;
}

EGLDisplay DrmBackend::eglDisplay() const
{
    return m_eglDisplay;
}

EGLNativeDisplayType DrmBackend::platformDisplay() const
{
    if (primaryDevice())
        return reinterpret_cast<EGLNativeDisplayType>(primaryDevice()->gbmDevice());
    return EGL_CAST(EGLNativeDisplayType, 0);
}

bool DrmBackend::isAtomicEnabled() const
{
    return m_enableAtomic;
}

void DrmBackend::initialize()
{
    if (m_initialized)
        return;

    m_initialized = true;

    // Session
    connect(m_session, &Session::devicePaused, this, [this](dev_t deviceId) {
        if (auto *device = findDevice(deviceId))
            device->setActive(false);
    });
    connect(m_session, &Session::deviceResumed, this, [this](dev_t deviceId) {
        if (auto *device = findDevice(deviceId))
            device->setActive(true);
    });

    // Find all GPUs
    if (!m_explicitDevices.isEmpty()) {
        for (const QString &fileName : m_explicitDevices)
            addDevice(fileName);
    } else {
        UdevEnumerate enumerate(UdevDevice::PrimaryVideoDevice | UdevDevice::GenericVideoDevice,
                                m_udev);
        const auto udevDevices = enumerate.scan();

        if (!udevDevices.isEmpty()) {
            qCDebug(gLcDrm, "Found the following video devices for the \"%s\" seat:",
                    qPrintable(m_session->seat()));
            for (const auto &udevDevice : udevDevices) {
                if (udevDevice->seat() == m_session->seat()) {
                    const auto path = udevDevice->deviceNode();
                    qCDebug(gLcDrm, "\t%s", qPrintable(path));
                    addDevice(path);
                }
            }
        }
    }

    if (Q_UNLIKELY(m_devices.empty()))
        qCFatal(gLcDrm, "No suitable DRM device have been found");

    // Create outputs
    updateOutputs();

    QCoreApplication::exit(1);
    return;

    // Monitor devices
    if (m_udevMonitor)
        m_udevMonitor->filterSubSystemDevType("drm"_L1);

    // Create EGL display
    createDisplay();
}

void DrmBackend::destroy()
{
}

DrmDevice *DrmBackend::primaryDevice() const
{
    return m_devices.empty() ? nullptr : m_devices.front().get();
}

Outputs DrmBackend::outputs() const
{
    return m_outputs;
}

DrmBackend *DrmBackend::instance()
{
    return gDrmBackend();
}

DrmDevice *DrmBackend::addDevice(const QString &path)
{
    // Open the DRM device
    int fd = m_session->openRestricted(path);
    if (fd < 0) {
        qCWarning(gLcDrm) << "Could not open DRM device" << path;
        return nullptr;
    }

    // Check whether DRM node supports KMS
    if (!drmIsKMS(fd)) {
        qCWarning(gLcDrm) << "Skipping KMS incapable DRM device" << path;
        m_session->closeRestricted(fd);
        return nullptr;
    }

    // Get the device ID
    struct stat sbuf;
    if (fstat(fd, &sbuf) < 0) {
        qCWarning(gLcDrm, "Failed to fstat \"%s\": %s", qPrintable(path), strerror(errno));
        m_session->closeRestricted(fd);
        return nullptr;
    }

    m_devices.push_back(std::make_unique<DrmDevice>(path, fd, sbuf.st_rdev));
    auto *device = m_devices.back().get();
    connect(device, &DrmDevice::outputAdded, this, &DrmBackend::addOutput);
    connect(device, &DrmDevice::outputRemoved, this, &DrmBackend::removeOutput);
    Q_EMIT deviceAdded(device);
    return device;
}

DrmDevice *DrmBackend::findDevice(dev_t deviceId) const
{
    auto it = std::find_if(m_devices.begin(), m_devices.end(), [deviceId](const auto &device) {
        return device->deviceId() == deviceId;
    });
    return it == m_devices.end() ? nullptr : it->get();
}

void DrmBackend::createDisplay()
{
    EGLNativeDisplayType nativeDisplay = platformDisplay();

    PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay = nullptr;
    const char *extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (extensions
        && (strstr(extensions, "EGL_KHR_platform_gbm")
            || strstr(extensions, "EGL_MESA_platform_gbm"))) {
        getPlatformDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
                eglGetProcAddress("eglGetPlatformDisplayEXT"));
    }

    if (getPlatformDisplay) {
        m_eglDisplay = getPlatformDisplay(EGL_PLATFORM_GBM_KHR, nativeDisplay, nullptr);
    } else {
        qCDebug(gLcDrm, "No eglGetPlatformDisplay for GBM, falling back to eglGetDisplay");
        m_eglDisplay = eglGetDisplay(nativeDisplay);
    }
}

void DrmBackend::updateOutputs()
{
    for (auto it = m_devices.begin(); it != m_devices.end(); ++it) {
        if ((*it)->isRemoved())
            (*it)->removeOutputs();
        else
            (*it)->updateOutputs();
    }

    Q_EMIT outputsQueried();

    for (auto it = m_devices.begin(); it != m_devices.end();) {
        auto *device = it->get();

        if (device->isRemoved() || (device != primaryDevice() && device->drmOutputs().isEmpty())) {
            qCDebug(gLcDrm, "Removing device \"%s\"", qPrintable(device->deviceNode()));
            const std::unique_ptr<DrmDevice> keepAlive = std::move(*it);
            it = m_devices.erase(it);
            Q_EMIT deviceRemoved(keepAlive.get());
        } else {
            it++;
        }
    }
}

void DrmBackend::handleDeviceAdded(Aurora::Platform::UdevDevice *udevDevice)
{
    if (!isUdevDeviceValid(udevDevice))
        return;

    auto *device = findDevice(udevDevice->deviceId());
    if (device) {
        qCWarning(gLcDrm) << "Received unexpected add udev event for:" << udevDevice->deviceNode();
        return;
    }

    if (addDevice(udevDevice->deviceNode()))
        updateOutputs();
}

void DrmBackend::handleDeviceRemoved(Aurora::Platform::UdevDevice *udevDevice)
{
    if (!isUdevDeviceValid(udevDevice))
        return;

    auto *device = findDevice(udevDevice->deviceId());
    if (device) {
        if (primaryDevice() == device) {
            qCCritical(gLcDrm, "Primary device has been removed, cannot continue!");
            QCoreApplication::exit(1);
            return;
        } else {
            device->setRemoved();
            updateOutputs();
        }
    }
}

void DrmBackend::handleDeviceChanged(Aurora::Platform::UdevDevice *udevDevice)
{
    if (!isUdevDeviceValid(udevDevice))
        return;

    auto *device = findDevice(udevDevice->deviceId());
    if (!device)
        device = addDevice(udevDevice->deviceNode());
    if (device && device->isActive()) {
        qCDebug(gLcDrm) << "Received change event for monitored drm device" << device->deviceNode();
        updateOutputs();
    }
}

void DrmBackend::addOutput(DrmOutput *output)
{
    m_outputs.append(output);
    Q_EMIT outputAdded(output);
}

void DrmBackend::removeOutput(DrmOutput *output)
{
    m_outputs.removeOne(output);
    Q_EMIT outputRemoved(output);
}

bool DrmBackend::isUdevDeviceValid(Aurora::Platform::UdevDevice *udevDevice)
{
    // Check the seat (but only if the device was not forced with AURORA_DRM_DEVICES)
    if (Q_LIKELY(m_explicitDevices.isEmpty())) {
        if (udevDevice->seat() == m_session->seat())
            return false;
    } else {
        const auto canonicalPath = QFileInfo(udevDevice->deviceNode()).canonicalPath();
        return std::any_of(m_explicitDevices.begin(), m_explicitDevices.end(),
                           [&canonicalPath](const QString &explicitPath) {
                               return QFileInfo(explicitPath).canonicalPath() == canonicalPath;
                           });
    }

    return true;
}

} // namespace Platform

} // namespace Aurora
