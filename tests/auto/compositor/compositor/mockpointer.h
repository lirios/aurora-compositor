// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include <QObject>
#include "wayland-wayland-client-protocol.h"

namespace Aurora {

namespace Compositor {

class MockPointer : public QObject
{
    Q_OBJECT

public:
    MockPointer(wl_seat *seat);
    ~MockPointer() override;

    wl_pointer *m_pointer = nullptr;
    wl_surface *m_enteredSurface = nullptr;
};

} // namespace Compositor

} // namespace Aurora

