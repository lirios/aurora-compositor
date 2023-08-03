// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QList>
#include <QSurfaceFormat>

#include <EGL/egl.h>

namespace Aurora {

namespace Platform {

bool hasEglExtension(EGLDisplay display, const char *name);
QVector<EGLint> eglConfigAttributesFromSurfaceFormat(EGLDisplay display,
                                                     const QSurfaceFormat &format);

} // namespace Platform

} // namespace Aurora
