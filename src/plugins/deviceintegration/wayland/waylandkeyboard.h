// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <LiriAuroraPlatform/KeyboardDevice>

#include <KWayland/Client/seat.h>
#include <KWayland/Client/keyboard.h>

namespace Aurora {

namespace Platform {

class WaylandKeyboard : public KeyboardDevice
{
    Q_OBJECT
public:
    explicit WaylandKeyboard(KWayland::Client::Seat *hostSeat, QObject *parent = nullptr);

    QString seatName() const override;

private:
    KWayland::Client::Seat *const m_hostSeat = nullptr;
    KWayland::Client::Keyboard *m_hostKeyboard = nullptr;
};

} // namespace Platform

} // namespace Aurora
