// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <QObject>
#include "wayland-wayland-client-protocol.h"

namespace Aurora {

namespace Compositor {

class MockKeyboard : public QObject
{
    Q_OBJECT

public:
    explicit MockKeyboard(wl_seat *seat);
    ~MockKeyboard() override;

    wl_keyboard *m_keyboard = nullptr;
    wl_surface *m_enteredSurface = nullptr;
    uint m_lastKeyCode = 0;
    uint m_lastKeyState = 0;
    uint m_group = 0;
};

} // namespace Compositor

} // namespace Aurora

