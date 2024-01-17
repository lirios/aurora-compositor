// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>
#include <QString>

#include <EGL/egl.h>

extern "C" {

void logGLInfo();
void logEGLInfo(EGLDisplay display);
void logEGLConfigInfo(EGLDisplay display, EGLConfig config);
}
