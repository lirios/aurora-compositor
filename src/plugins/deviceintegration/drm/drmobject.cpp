// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2016 Roman Gilg <subdiff@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "drmobject.h"
#include "drmdevice.h"
#include "drmloggingcategories.h"
#include "drmpointer.h"

#include <errno.h>

namespace Aurora {

namespace Platform {

DrmObject::DrmObject(DrmDevice *device, uint32_t objectId, uint32_t objectType)
    : m_device(device)
    , m_id(objectId)
    , m_objectType(objectType)
{
}

bool DrmObject::init()
{
    return updateProperties();
}

DrmPropertyList DrmObject::queryProperties() const
{
    DrmUniquePtr<drmModeObjectProperties> properties(
            drmModeObjectGetProperties(m_device->fd(), m_id, m_objectType));
    if (!properties) {
        qCWarning(gLcDrm) << "Failed to get properties for object" << m_id;
        return {};
    }
    DrmPropertyList ret;
    for (uint32_t i = 0; i < properties->count_props; i++) {
        DrmUniquePtr<drmModePropertyRes> prop(
                drmModeGetProperty(m_device->fd(), properties->props[i]));
        if (!prop) {
            qCWarning(gLcDrm, "Getting property %d of object %d failed!", properties->props[i],
                      m_id);
            continue;
        }
        ret.addProperty(std::move(prop), properties->prop_values[i]);
    }
    return ret;
}

uint32_t DrmObject::id() const
{
    return m_id;
}

DrmDevice *DrmObject::device() const
{
    return m_device;
}

uint32_t DrmObject::type() const
{
    return m_objectType;
}

QString DrmObject::typeName() const
{
    switch (m_objectType) {
    case DRM_MODE_OBJECT_CONNECTOR:
        return QStringLiteral("connector");
    case DRM_MODE_OBJECT_CRTC:
        return QStringLiteral("crtc");
    case DRM_MODE_OBJECT_PLANE:
        return QStringLiteral("plane");
    default:
        return QStringLiteral("unknown?");
    }
}

void DrmPropertyList::addProperty(DrmUniquePtr<drmModePropertyRes> &&prop, uint64_t value)
{
    m_properties.push_back(std::make_pair(std::move(prop), value));
}

std::optional<std::pair<DrmUniquePtr<drmModePropertyRes>, uint64_t>>
DrmPropertyList::takeProperty(const QByteArray &name)
{
    const auto it = std::find_if(m_properties.begin(), m_properties.end(),
                                 [&name](const auto &pair) { return pair.first->name == name; });
    if (it != m_properties.end()) {
        auto ret = std::move(*it);
        m_properties.erase(it);
        return ret;
    } else {
        return std::nullopt;
    }
}

} // namespace Platform

} // namespace Aurora

QDebug operator<<(QDebug s, const Aurora::Platform::DrmObject *obj)
{
    QDebugStateSaver saver(s);
    if (obj)
        s.nospace() << "DrmObject(id=" << obj->id() << ", device=" << obj->device() << ')';
    else
        s << "DrmObject(0x0)";
    return s;
}
