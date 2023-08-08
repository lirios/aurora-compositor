// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPointer>

#include <LiriAuroraPlatform/TouchDevice>

#include <KWayland/Client/seat.h>
#include <KWayland/Client/touch.h>

namespace Aurora {

namespace Platform {

class WaylandTouch : public TouchDevice
{
    Q_OBJECT
public:
    explicit WaylandTouch(KWayland::Client::Seat *hostSeat, QObject *parent = nullptr);

    QString seatName() const override;

private:
    KWayland::Client::Seat *const m_hostSeat = nullptr;
    KWayland::Client::Touch *m_hostTouch = nullptr;
};

} // namespace Platform

} // namespace Aurora
