// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2016 Roman Gilg <subdiff@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QByteArray>
#include <QList>
#include <QMap>

#include <vector>

#include <xf86drmMode.h>

#include "drmpointer.h"
#include "drmproperty.h"

namespace Aurora {

namespace Platform {

class DrmBackend;
class DrmDevice;
class DrmOutput;
class DrmAtomicCommit;

class DrmPropertyList
{
public:
    void addProperty(DrmUniquePtr<drmModePropertyRes> &&prop, uint64_t value);
    std::optional<std::pair<DrmUniquePtr<drmModePropertyRes>, uint64_t>>
    takeProperty(const QByteArray &name);

private:
    std::vector<std::pair<DrmUniquePtr<drmModePropertyRes>, uint64_t>> m_properties;
};

class DrmObject
{
public:
    virtual ~DrmObject() = default;
    DrmObject(const DrmObject &) = delete;

    /**
     * Must be called to query necessary data directly after creation.
     * @return true when initializing was successful
     */
    bool init();

#if 0
    /**
     * Set the properties in such a way that this resource won't be used anymore
     */
    virtual void disable(DrmAtomicCommit *commit) = 0;
#endif

    virtual bool updateProperties() = 0;

    uint32_t id() const;
    DrmDevice *device() const;
    uint32_t type() const;
    QString typeName() const;

protected:
    DrmObject(DrmDevice *device, uint32_t objectId, uint32_t objectType);

    DrmPropertyList queryProperties() const;

private:
    DrmDevice *m_device;
    const uint32_t m_id;
    const uint32_t m_objectType;
};

} // namespace Platform

} // namespace Aurora

QDebug operator<<(QDebug stream, const Aurora::Platform::DrmObject *);
