// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "waylandtouch.h"

namespace Aurora {

namespace Platform {

WaylandTouch::WaylandTouch(KWayland::Client::Seat *hostSeat, QObject *parent)
    : TouchDevice(parent)
    , m_hostSeat(hostSeat)
    , m_hostTouch(hostSeat->createTouch(this))
{
}

QString WaylandTouch::seatName() const
{
    return m_hostSeat->name();
}

} // namespace Platform

} // namespace Aurora
