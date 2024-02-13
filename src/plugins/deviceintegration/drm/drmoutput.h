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
    explicit DrmOutput(const std::shared_ptr<DrmConnector> &connector, QObject *parent = nullptr);
    ~DrmOutput() override;

    DrmConnector *connector() const;

    void destroy();

private:
    const std::shared_ptr<DrmConnector> m_connector;
};

} // namespace Platform

} // namespace Aurora
