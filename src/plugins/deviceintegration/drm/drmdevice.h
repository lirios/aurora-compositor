// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QObject>
#include <QList>

#include <gbm.h>

#include "drmeventreader.h"
#include "filedescriptor.h"

namespace Aurora {

namespace Platform {

class DrmConnector;
class DrmCrtc;
class DrmObject;
class DrmOutput;
class DrmPlane;

class DrmDevice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
public:
    enum class DrmDriver {
        Unknown = 0,
        I915,
        NVidia,
        VirtualMachine
    };
    Q_ENUM(DrmDriver)

    explicit DrmDevice(const QString &path, int fd, dev_t deviceId, QObject *parent = nullptr);
    ~DrmDevice();

    QString deviceNode() const;
    int fd() const;
    dev_t deviceId() const;

    gbm_device *gbmDevice() const;

    bool isActive() const;
    void setActive(bool active);

    bool isRemoved() const;
    void setRemoved();

    bool hasAtomicSupport() const;
    bool hasAddFB2ModifiersSupport() const;

    bool updateOutputs();
    void removeOutputs();

    QList<DrmOutput *> drmOutputs() const;

Q_SIGNALS:
    void activeChanged(bool active);
    void outputAdded(DrmOutput *output);
    void outputRemoved(DrmOutput *output);

private:
    void initDrmResources();

    const QString m_path;
    const int m_fd = -1;
    const dev_t m_deviceId = 0;

    FileDescriptor m_gbmFd;
    gbm_device *m_gbmDevice = nullptr;

    DrmDriver m_drmDriver = DrmDriver::Unknown;

    DrmEventReader m_eventReader;

    bool m_active = true;
    bool m_removed = false;
    bool m_hasAtomicSupport = false;
    bool m_hasAddFB2ModifiersSupport = false;

    QList<DrmObject *> m_allObjects;
    std::vector<std::unique_ptr<DrmPlane>> m_planes;
    std::vector<std::unique_ptr<DrmCrtc>> m_crtcs;
    std::vector<std::shared_ptr<DrmConnector>> m_connectors;

    QList<DrmOutput *> m_drmOutputs;
};

} // namespace Platform

} // namespace Aurora
