// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2023 Xaver Hugl <xaver.hugl@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <memory>
#include <stdint.h>

namespace Aurora {

namespace Platform {

class DrmDevice;

class DrmBlob
{
public:
    explicit DrmBlob(DrmDevice *device, uint32_t blobId);
    ~DrmBlob();

    uint32_t blobId() const;

    static std::shared_ptr<DrmBlob> create(DrmDevice *device, const void *data, uint32_t dataSize);

protected:
    DrmDevice *const m_device;
    const uint32_t m_blobId;
};

} // namespace Platform

} // namespace Aurora
