// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drmoutput.h"
#include "drmwindow.h"

namespace Aurora {

namespace Platform {

DrmWindow::DrmWindow(DrmOutput *output, QWindow *qtWindow, QObject *parent)
    : Window(output, qtWindow, parent)
{
}

bool DrmWindow::create()
{
    return false;
}

void DrmWindow::destroy()
{
}

} // namespace Platform

} // namespace Aurora
