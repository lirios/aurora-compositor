// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <LiriAuroraPlatform/PointerDevice>

#include <KWayland/Client/seat.h>
#include <KWayland/Client/pointer.h>

namespace Aurora {

namespace Platform {

class WaylandPointer : public PointerDevice
{
    Q_OBJECT
public:
    explicit WaylandPointer(KWayland::Client::Seat *hostSeat, QObject *parent = nullptr);

    QString seatName() const override;

private:
    KWayland::Client::Seat *const m_hostSeat = nullptr;
    KWayland::Client::Pointer *m_hostPointer = nullptr;
    QPointer<KWayland::Client::Surface> m_enteredSurface;

    Qt::MouseButton buttonToQtMouseButton(quint32 button);
};

} // namespace Platform

} // namespace Aurora
