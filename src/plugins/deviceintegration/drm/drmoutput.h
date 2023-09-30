// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <LiriAuroraPlatform/Output>

namespace Aurora {

namespace Platform {

class DrmConnector;

class DrmOutput : public Output
{
    Q_OBJECT
public:
    explicit DrmOutput(QObject *parent = nullptr);
    ~DrmOutput() override;

    QString name() const override;
    QString description() const override;

    void destroy();
};

} // namespace Platform

} // namespace Aurora
