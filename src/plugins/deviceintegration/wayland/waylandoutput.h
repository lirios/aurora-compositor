// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <LiriAuroraPlatform/Output>

#include <KWayland/Client/output.h>

namespace Aurora {

namespace Platform {

class WaylandIntegration;

class WaylandOutput : public Output
{
    Q_OBJECT
public:
    explicit WaylandOutput(KWayland::Client::Output *output, const QString &name,
                           QObject *parent = nullptr);
    ~WaylandOutput();

    void destroy();

protected:
    KWayland::Client::Output *m_output = nullptr;

private:
    QString m_name;

private slots:
    void updateInfo();
    void handleModeAdded(const KWayland::Client::Output::Mode &mode);
    void handleModeChanged(const KWayland::Client::Output::Mode &mode);
};

} // namespace Platform

} // namespace Aurora
