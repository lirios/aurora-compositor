// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <LiriAuroraPlatform/Output>
#include <LiriAuroraPlatform/OutputsModel>

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

class LIRIAURORAPLATFORM_EXPORT OutputsModelPrivate
{
    Q_DECLARE_PUBLIC(OutputsModel)
public:
    OutputsModelPrivate(OutputsModel *self);

    QList<Output *> outputs;

protected:
    OutputsModel *q_ptr = nullptr;
};

} // namespace Platform

} // namespace Aurora
