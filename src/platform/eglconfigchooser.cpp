// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "eglconfigchooser_p.h"

#ifndef EGL_OPENGL_ES3_BIT_KHR
#  define EGL_OPENGL_ES3_BIT_KHR 0x0040
#endif

namespace Aurora {

namespace Platform {

bool hasEglExtension(EGLDisplay display, const char *name)
{
    QList<QByteArray> extensions =
            QByteArray(reinterpret_cast<const char *>(eglQueryString(display, EGL_EXTENSIONS)))
                    .split(' ');
    return extensions.contains(name);
}

QVector<EGLint> eglConfigAttributesFromSurfaceFormat(EGLDisplay display,
                                                     const QSurfaceFormat &format)
{
    QVector<EGLint> configAttribs;

    configAttribs.append(EGL_RED_SIZE);
    configAttribs.append(qMax<int>(0, format.redBufferSize()));

    configAttribs.append(EGL_GREEN_SIZE);
    configAttribs.append(qMax<int>(0, format.greenBufferSize()));

    configAttribs.append(EGL_BLUE_SIZE);
    configAttribs.append(qMax<int>(0, format.blueBufferSize()));

    configAttribs.append(EGL_ALPHA_SIZE);
    configAttribs.append(qMax<int>(0, format.alphaBufferSize()));

    configAttribs.append(EGL_SAMPLES);
    configAttribs.append(qMax<int>(0, format.samples()));

    configAttribs.append(EGL_SAMPLE_BUFFERS);
    configAttribs.append(format.samples() > 0 ? 1 : 0);

    switch (format.renderableType()) {
    case QSurfaceFormat::OpenGL:
        configAttribs.append(EGL_RENDERABLE_TYPE);
        configAttribs.append(EGL_OPENGL_BIT);
        break;
    case QSurfaceFormat::OpenGLES:
        configAttribs.append(EGL_RENDERABLE_TYPE);
        if (format.majorVersion() == 1)
            configAttribs.append(EGL_OPENGL_ES_BIT);
        else if (format.majorVersion() == 2)
            configAttribs.append(EGL_OPENGL_ES2_BIT);
        else if (format.majorVersion() == 3 && hasEglExtension(display, "EGL_KHR_create_context"))
            configAttribs.append(EGL_OPENGL_ES3_BIT_KHR);
        else if (format.majorVersion() == 3)
            configAttribs.append(EGL_OPENGL_ES3_BIT);
        break;
    case QSurfaceFormat::OpenVG:
        configAttribs.append(EGL_RENDERABLE_TYPE);
        configAttribs.append(EGL_OPENVG_BIT);
        break;
    default:
        break;
    }

    if (format.renderableType() != QSurfaceFormat::OpenVG) {
        configAttribs.append(EGL_DEPTH_SIZE);
        configAttribs.append(qMax<int>(0, format.depthBufferSize()));

        configAttribs.append(EGL_STENCIL_SIZE);
        configAttribs.append(qMax<int>(0, format.stencilBufferSize()));
    } else {
        configAttribs.append(EGL_ALPHA_MASK_SIZE);
        configAttribs.append(8);
    }

    return configAttribs;
}

} // namespace Platform

} // namespace Aurora
