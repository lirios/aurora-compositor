// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPointF>

#include <LiriAuroraPlatform/InputManager>

class EglFSInputManager : public QObject
{
    Q_OBJECT
public:
    explicit EglFSInputManager(QObject *parent = nullptr);

private:
    Aurora::Platform::InputManager *m_inputManager = nullptr;
    Qt::MouseButtons m_buttons = Qt::NoButton;
    QPointF m_pos;

private slots:
    void handleInputDeviceAddedOrRemoved();
    void handleKeyboardAdded(Aurora::Platform::KeyboardDevice *keyboard);
    void handlePointerAdded(Aurora::Platform::PointerDevice *pointer);
    void handleTouchAdded(Aurora::Platform::TouchDevice *touch);
};
