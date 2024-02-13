// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include <LiriAuroraPlatform/DeviceIntegration>
#include <LiriAuroraPlatform/Session>

#include <EGL/egl.h>

#include "drmoutput.h"

class QSocketNotifier;

namespace Aurora {

namespace Platform {

class DrmDevice;
class DrmOutput;
class Udev;
class UdevDevice;
class UdevMonitor;

class DrmBackend : public QObject
{
    Q_OBJECT
public:
    explicit DrmBackend(QObject *parent = nullptr);
    ~DrmBackend();

    Session *session() const;

    EGLDisplay eglDisplay() const;
    EGLNativeDisplayType platformDisplay() const;

    bool isAtomicEnabled() const;

    void initialize();
    void destroy();

    DrmDevice *primaryDevice() const;

    Outputs outputs() const;

    static DrmBackend *instance();

Q_SIGNALS:
    void statusChanged(DeviceIntegration::Status status);
    void deviceAdded(DrmDevice *device);
    void deviceRemoved(DrmDevice *device);
    void outputAdded(DrmOutput *output);
    void outputRemoved(DrmOutput *output);
    void outputsQueried();

private:
    bool m_initialized = false;

    Session *m_session = nullptr;
    Udev *m_udev = nullptr;
    UdevMonitor *m_udevMonitor = nullptr;

    EGLDisplay m_eglDisplay = EGL_NO_DISPLAY;
    bool m_enableAtomic = true;

    const QStringList m_explicitDevices;
    std::vector<std::unique_ptr<DrmDevice>> m_devices;

    Outputs m_outputs;

    DrmDevice *findDevice(dev_t deviceId) const;
    DrmDevice *addDevice(const QString &path);

    void createDisplay();
    void updateOutputs();

private Q_SLOTS:
    void handleDeviceAdded(Aurora::Platform::UdevDevice *udevDevice);
    void handleDeviceRemoved(Aurora::Platform::UdevDevice *udevDevice);
    void handleDeviceChanged(Aurora::Platform::UdevDevice *udevDevice);
    void addOutput(DrmOutput *output);
    void removeOutput(DrmOutput *output);

private:
    bool isUdevDeviceValid(Aurora::Platform::UdevDevice *udevDevice);
};

} // namespace Platform

} // namespace Aurora
