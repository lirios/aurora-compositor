// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/private/qinputdevicemanager_p_p.h>

#include <qpa/qwindowsysteminterface.h>

#include <LiriAuroraPlatform/KeyboardDevice>
#include <LiriAuroraPlatform/PointerDevice>
#include <LiriAuroraPlatform/TouchDevice>
#include <LiriAuroraPlatform/private/eglfsdeviceintegration_p.h>

#include "eglfsinputmanager.h"

EglFSInputManager::EglFSInputManager(QObject *parent)
    : QObject(parent)
    , m_inputManager(Aurora::Platform::auroraDeviceIntegration()->createInputManager(this))
{
    const auto keyboards = m_inputManager->keyboardDevices();
    for (auto *keyboard : keyboards)
        handleKeyboardAdded(keyboard);

    const auto pointers = m_inputManager->pointerDevices();
    for (auto *pointer : pointers)
        handlePointerAdded(pointer);

    const auto touchs = m_inputManager->touchDevices();
    for (auto *touch : touchs)
        handleTouchAdded(touch);

    connect(m_inputManager, &Aurora::Platform::InputManager::keyboardAdded, this,
            &EglFSInputManager::handleKeyboardAdded);
    connect(m_inputManager, &Aurora::Platform::InputManager::pointerAdded, this,
            &EglFSInputManager::handlePointerAdded);
    connect(m_inputManager, &Aurora::Platform::InputManager::touchAdded, this,
            &EglFSInputManager::handleTouchAdded);
}

void EglFSInputManager::handleInputDeviceAddedOrRemoved()
{
    QInputDeviceManager *inputManager = QGuiApplicationPrivate::inputDeviceManager();
    QInputDeviceManagerPrivate *inputManagerPriv = QInputDeviceManagerPrivate::get(inputManager);

    inputManagerPriv->setDeviceCount(
            QInputDeviceManager::DeviceTypePointer,
            m_inputManager->deviceCount(Aurora::Platform::InputDevice::DeviceType::Pointer));
    inputManagerPriv->setDeviceCount(
            QInputDeviceManager::DeviceTypeKeyboard,
            m_inputManager->deviceCount(Aurora::Platform::InputDevice::DeviceType::Keyboard));
    inputManagerPriv->setDeviceCount(
            QInputDeviceManager::DeviceTypeTouch,
            m_inputManager->deviceCount(Aurora::Platform::InputDevice::DeviceType::Touch));
    inputManagerPriv->setDeviceCount(
            QInputDeviceManager::DeviceTypeTablet,
            m_inputManager->deviceCount(Aurora::Platform::InputDevice::DeviceType::Tablet));
}

void EglFSInputManager::handleKeyboardAdded(Aurora::Platform::KeyboardDevice *keyboard)
{
    connect(keyboard, &Aurora::Platform::KeyboardDevice::keyPressed, this,
            [](const Aurora::Platform::KeyboardDevice::KeyEvent &keyEvent) {
                QWindowSystemInterface::handleExtendedKeyEvent(
                        nullptr, keyEvent.timestamp, QEvent::KeyPress, keyEvent.key,
                        keyEvent.modifiers, keyEvent.nativeScanCode, keyEvent.nativeVirtualKey,
                        keyEvent.nativeModifiers, keyEvent.text, keyEvent.autoRepeat,
                        keyEvent.repeatCount);
            });
    connect(keyboard, &Aurora::Platform::KeyboardDevice::keyReleased, this,
            [](const Aurora::Platform::KeyboardDevice::KeyEvent &keyEvent) {
                QWindowSystemInterface::handleExtendedKeyEvent(
                        nullptr, keyEvent.timestamp, QEvent::KeyRelease, keyEvent.key,
                        keyEvent.modifiers, keyEvent.nativeScanCode, keyEvent.nativeVirtualKey,
                        keyEvent.nativeModifiers, keyEvent.text, keyEvent.autoRepeat,
                        keyEvent.repeatCount);
            });
}

void EglFSInputManager::handlePointerAdded(Aurora::Platform::PointerDevice *pointer)
{
    connect(pointer, &Aurora::Platform::PointerDevice::motion, this,
            [this](const QPointF &absPosition) {
                Q_UNUSED(time)

                m_pos = absPosition;

                Qt::KeyboardModifiers mods =
                        QGuiApplicationPrivate::inputDeviceManager()->keyboardModifiers();
                QWindowSystemInterface::handleMouseEvent(nullptr, absPosition, absPosition,
                                                         m_buttons, Qt::NoButton, QEvent::MouseMove,
                                                         mods);
            });
    connect(pointer, &Aurora::Platform::PointerDevice::buttonPressed, this,
            [this](Qt::MouseButton button) {
                Q_UNUSED(time)

                m_buttons.setFlag(button, true);

                Qt::KeyboardModifiers mods =
                        QGuiApplicationPrivate::inputDeviceManager()->keyboardModifiers();
                QWindowSystemInterface::handleMouseEvent(nullptr, m_pos, m_pos, m_buttons, button,
                                                         QEvent::MouseButtonPress, mods);
            });
    connect(pointer, &Aurora::Platform::PointerDevice::buttonReleased, this,
            [this](Qt::MouseButton button) {
                Q_UNUSED(time)

                m_buttons.setFlag(button, false);

                Qt::KeyboardModifiers mods =
                        QGuiApplicationPrivate::inputDeviceManager()->keyboardModifiers();
                QWindowSystemInterface::handleMouseEvent(nullptr, m_pos, m_pos, m_buttons, button,
                                                         QEvent::MouseButtonRelease, mods);
            });
}

void EglFSInputManager::handleTouchAdded(Aurora::Platform::TouchDevice *touch)
{
    Q_UNUSED(touch);
}
