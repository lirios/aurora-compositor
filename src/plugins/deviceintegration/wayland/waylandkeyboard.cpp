// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "waylandkeyboard.h"

namespace Aurora {

namespace Platform {

WaylandKeyboard::WaylandKeyboard(KWayland::Client::Seat *hostSeat, QObject *parent)
    : KeyboardDevice(parent)
    , m_hostSeat(hostSeat)
    , m_hostKeyboard(hostSeat->createKeyboard(this))
{
    connect(m_hostKeyboard, &KWayland::Client::Keyboard::keymapChanged, this,
            [this](int fd, quint32 size) { handleKeymapChanged(fd, size); });
    connect(m_hostKeyboard, &KWayland::Client::Keyboard::keyChanged, this,
            [this](quint32 key, KWayland::Client::Keyboard::KeyState state, quint32 time) {
                handleKeyChanged(key,
                                 state == KWayland::Client::Keyboard::KeyState::Pressed
                                         ? KeyState::Pressed
                                         : KeyState::Released,
                                 time);
            });
    connect(m_hostKeyboard, &KWayland::Client::Keyboard::modifiersChanged, this,
            [this](quint32 depressed, quint32 latched, quint32 locked, quint32 group) {
                handleModifiers(depressed, latched, locked, group);
            });
    connect(m_hostKeyboard, &KWayland::Client::Keyboard::keyRepeatChanged, this, [this]() {
        setKeyRepeatEnabled(true);
        setKeyRepeatRate(m_hostKeyboard->keyRepeatRate());
        setKeyRepeatDelay(m_hostKeyboard->keyRepeatDelay());
    });
}

QString WaylandKeyboard::seatName() const
{
    return m_hostSeat->name();
}

} // namespace Platform

} // namespace Aurora
