// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWindow>

#include <LiriAuroraPlatform/Window>

namespace Aurora {

namespace Platform {

class DrmOutput;

class DrmWindow : public Window
{
    Q_OBJECT
public:
    explicit DrmWindow(DrmOutput *output, QWindow *qtWindow, QObject *parent = nullptr);

    bool create() override;
    void destroy() override;
};

} // namespace Platform

} // namespace Aurora
