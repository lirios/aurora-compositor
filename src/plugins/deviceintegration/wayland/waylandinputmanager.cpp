// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "waylandbackend.h"
#include "waylandinputmanager.h"
#include "waylandkeyboard.h"
#include "waylandpointer.h"
#include "waylandtouch.h"

namespace Aurora {

namespace Platform {

WaylandInputManager::WaylandInputManager(QObject *parent)
    : InputManager(parent)
{
    auto *seat = WaylandBackend::instance()->seat();

    handleHasKeyboardChanged(seat->hasKeyboard());
    handleHasPointerChanged(seat->hasPointer());
    handleHasTouchChanged(seat->hasTouch());

    connect(seat, &KWayland::Client::Seat::hasKeyboardChanged, this,
            &WaylandInputManager::handleHasKeyboardChanged);
    connect(seat, &KWayland::Client::Seat::hasPointerChanged, this,
            &WaylandInputManager::handleHasPointerChanged);
    connect(seat, &KWayland::Client::Seat::hasTouchChanged, this,
            &WaylandInputManager::handleHasTouchChanged);
}

WaylandInputManager::~WaylandInputManager()
{
}

QList<KeyboardDevice *> WaylandInputManager::keyboardDevices() const
{
    return QList<KeyboardDevice *>() << m_keyboard;
}

QList<PointerDevice *> WaylandInputManager::pointerDevices() const
{
    return QList<PointerDevice *>() << m_pointer;
}

QList<TouchDevice *> WaylandInputManager::touchDevices() const
{
    return QList<TouchDevice *>() << m_touch;
}

int WaylandInputManager::deviceCount(InputDevice::DeviceType deviceType) const
{
    switch (deviceType) {
    case InputDevice::DeviceType::Keyboard:
        return m_keyboard ? 1 : 0;
    case InputDevice::DeviceType::Pointer:
        return m_pointer ? 1 : 0;
    case InputDevice::DeviceType::Touch:
        return m_touch ? 1 : 0;
    default:
        return 0;
    }
}

void WaylandInputManager::handleHasKeyboardChanged(bool hasKeyboard)
{
    auto *seat = WaylandBackend::instance()->seat();

    if (hasKeyboard) {
        m_keyboard = new WaylandKeyboard(seat, this);
        Q_EMIT deviceAdded(m_keyboard);
        Q_EMIT keyboardAdded(m_keyboard);
    } else {
        if (m_keyboard) {
            Q_EMIT deviceRemoved(m_keyboard);
            Q_EMIT keyboardRemoved(m_keyboard);
            m_keyboard->deleteLater();
            m_keyboard = nullptr;
        }
    }
}

void WaylandInputManager::handleHasPointerChanged(bool hasPointer)
{
    auto *seat = WaylandBackend::instance()->seat();

    if (hasPointer) {
        m_pointer = new WaylandPointer(seat, this);
        Q_EMIT deviceAdded(m_pointer);
        Q_EMIT pointerAdded(m_pointer);
    } else {
        if (m_pointer) {
            Q_EMIT deviceRemoved(m_pointer);
            Q_EMIT pointerRemoved(m_pointer);
            m_pointer->deleteLater();
            m_pointer = nullptr;
        }
    }
}

void WaylandInputManager::handleHasTouchChanged(bool hasTouch)
{
    auto *seat = WaylandBackend::instance()->seat();

    if (hasTouch) {
        m_touch = new WaylandTouch(seat, this);
        Q_EMIT deviceAdded(m_touch);
        Q_EMIT touchAdded(m_touch);
    } else {
        if (m_pointer) {
            Q_EMIT deviceRemoved(m_touch);
            Q_EMIT touchRemoved(m_touch);
            m_touch->deleteLater();
            m_touch = nullptr;
        }
    }
}

} // namespace Platform

} // namespace Aurora
