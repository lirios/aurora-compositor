// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2023 Xaver Hugl <xaver.hugl@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "drmblob.h"
#include "drmdevice.h"

#include <xf86drmMode.h>

namespace Aurora {

namespace Platform {

DrmBlob::DrmBlob(DrmDevice *device, uint32_t blobId)
    : m_device(device)
    , m_blobId(blobId)
{
}

DrmBlob::~DrmBlob()
{
    if (m_blobId)
        drmModeDestroyPropertyBlob(m_device->fd(), m_blobId);
}

uint32_t DrmBlob::blobId() const
{
    return m_blobId;
}

std::shared_ptr<DrmBlob> DrmBlob::create(DrmDevice *device, const void *data, uint32_t dataSize)
{
    uint32_t id = 0;
    if (drmModeCreatePropertyBlob(device->fd(), data, dataSize, &id) == 0)
        return std::make_shared<DrmBlob>(device, id);
    else
        return nullptr;
}

} // namespace Platform

} // namespace Aurora
