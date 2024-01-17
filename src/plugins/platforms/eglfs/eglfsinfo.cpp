// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eglfscategories.h"
#include "eglfsinfo.h"

#include <GLES2/gl2.h>
#include <EGL/egl.h>

const char *tab = "                  ";

static void logGLextensions(const char *prefix, const QList<QByteArray> &extensions)
{
    QString extensionString;
    const int maxLineLength = 98;
    int currentLineLength = 0;

    for (int i = 0; i < extensions.size(); i++) {
        const auto extension = extensions[i];

        // Check if adding the current extension fits in the current line
        if (currentLineLength + extension.length() + 2 <= maxLineLength) { // 2 accounts for "  "
            if (!extensionString.isEmpty()) {
                extensionString += QStringLiteral("  ");
                currentLineLength += 2;
            }
            extensionString += QString::fromUtf8(extension);
            currentLineLength += extension.length();
        } else {
            extensionString += QStringLiteral("\n") + QString::fromUtf8(extension);
            currentLineLength = extension.length();
        }
    }

    if (!extensionString.isEmpty()) {
        const auto numSpaces = qMax<int>(0, 18 - strlen(prefix) - 1);
        auto lines = extensionString.split(QLatin1Char('\n'));

        for (int i = 0; i < lines.size(); i++) {
            const auto line = lines[i];
            if (i == 0)
                qCInfo(gLcEglFS, "%s:%*s%s", prefix, numSpaces, " ", qPrintable(line));
            else
                qCInfo(gLcEglFS, "%s%s", tab, qPrintable(line));
        }
    }
}

void logGLInfo()
{
    const char *str;

    str = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    qCInfo(gLcEglFS, "GL version:       %s", str ? str : "(null)");

    str = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    qCInfo(gLcEglFS, "GLSL version:     %s", str ? str : "(null)");

    str = reinterpret_cast<const char *>(glGetString(GL_VENDOR));
    qCInfo(gLcEglFS, "GL vendor:        %s", str ? str : "(null)");

    str = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
    qCInfo(gLcEglFS, "GL renderer:      %s", str ? str : "(null)");

    QList<QByteArray> extensions =
            QByteArray(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS))).split(' ');
    logGLextensions("GL extensions", extensions);
}

void logEGLInfo(EGLDisplay display)
{
    const char *str;

    str = eglQueryString(display, EGL_VERSION);
    qCInfo(gLcEglFS, "EGL version:      %s", str ? str : "(null)");

    str = eglQueryString(display, EGL_VENDOR);
    qCInfo(gLcEglFS, "EGL vendor:       %s", str ? str : "(null)");

    str = eglQueryString(display, EGL_CLIENT_APIS);
    qCInfo(gLcEglFS, "EGL client APIs:  %s", str ? str : "(null)");

    QList<QByteArray> extensions = QByteArray(eglQueryString(display, EGL_EXTENSIONS)).split(' ');
    logGLextensions("EGL extensions", extensions);
}

void logEGLConfigInfo(EGLDisplay display, EGLConfig config)
{
    if (!config)
        return;

    EGLint r, g, b, a;

    qCInfo(gLcEglFS, "EGL attributes:");

    if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &r)
        && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &g)
        && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &b)
        && eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &a))
        qCInfo(gLcEglFS, "%sRGBA bits: %d %d %d %d", tab, r, g, b, a);
    else
        qCInfo(gLcEglFS, "%sRGBA bits: unknown", tab);

    if (eglGetConfigAttrib(display, config, EGL_MIN_SWAP_INTERVAL, &a)
        && eglGetConfigAttrib(display, config, EGL_MAX_SWAP_INTERVAL, &b))
        qCInfo(gLcEglFS, "%sSwap interval range: %d - %d", tab, a, b);
    else
        qCInfo(gLcEglFS, "%sSwap interval range: unknown", tab);
}
