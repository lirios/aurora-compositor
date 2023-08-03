// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <LiriAuroraPlatform/DeviceIntegration>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Aurora API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace Aurora {

namespace Platform {

class LIRIAURORAPLATFORM_EXPORT DeviceIntegrationPrivate
{
    Q_DECLARE_PUBLIC(DeviceIntegration)
public:
    DeviceIntegrationPrivate(DeviceIntegration *self);

    DeviceIntegration::Status status = DeviceIntegration::Status::NotReady;

protected:
    DeviceIntegration *q_ptr = nullptr;
};

} // namespace Platform

} // namespace Aurora
