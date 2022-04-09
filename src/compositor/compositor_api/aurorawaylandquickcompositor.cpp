/****************************************************************************
**
** Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtQml/QQmlEngine>
#include <QQuickWindow>
#if QT_CONFIG(opengl)
#  include <QOpenGLTextureBlitter>
#  include <QOpenGLTexture>
#  include <QOpenGLFramebufferObject>
#endif
#include <QMatrix4x4>
#include <QRunnable>

#include "aurorawaylandclient.h"
#include "aurorawaylandquickcompositor.h"
#include "aurorawaylandquicksurface.h"
#include "aurorawaylandquickoutput.h"
#include "aurorawaylandquickitem.h"
#include "aurorawaylandoutput.h"
#include <LiriAuroraCompositor/private/aurorawaylandcompositor_p.h>
#include <LiriAuroraCompositor/WaylandViewporter>
#include "aurorawaylandsurfacegrabber.h"

namespace Aurora {

namespace Compositor {

class WaylandQuickCompositorPrivate : public WaylandCompositorPrivate
{
public:
    explicit WaylandQuickCompositorPrivate(WaylandCompositor *compositor)
        : WaylandCompositorPrivate(compositor)
        , m_viewporter(new WaylandViewporter(compositor))
    {
    }
protected:
    WaylandSurface *createDefaultSurface() override
    {
        return new WaylandQuickSurface();
    }
private:
    QScopedPointer<WaylandViewporter> m_viewporter;
};

WaylandQuickCompositor::WaylandQuickCompositor(QObject *parent)
    : WaylandCompositor(*new WaylandQuickCompositorPrivate(this), parent)
{
}

/*!
 * \qmlproperty list QtWaylandCompositor::WaylandCompositor::extensions
 *
 * A list of extensions that the compositor advertises to its clients. For
 * any Wayland extension the compositor should support, instantiate its component,
 * and add it to the list of extensions.
 *
 * For instance, the following code would allow the clients to request \c wl_shell
 * surfaces in the compositor using the \c wl_shell interface.
 *
 * \qml
 * import Aurora.Compositor
 *
 * WaylandCompositor {
 *     WlShell {
 *         // ...
 *     }
 * }
 * \endqml
 */

void WaylandQuickCompositor::create()
{
    WaylandCompositor::create();
}


void WaylandQuickCompositor::classBegin()
{
    WaylandCompositorPrivate::get(this)->preInit();
}

void WaylandQuickCompositor::componentComplete()
{
    create();
}

/*!
 * Grab the surface content from the given \a buffer.
 * Reimplemented from WaylandCompositor::grabSurface.
 */
void WaylandQuickCompositor::grabSurface(WaylandSurfaceGrabber *grabber, const WaylandBufferRef &buffer)
{
    if (buffer.isSharedMemory()) {
        WaylandCompositor::grabSurface(grabber, buffer);
        return;
    }

#if QT_CONFIG(opengl)
    WaylandQuickOutput *output = static_cast<WaylandQuickOutput *>(defaultOutput());
    if (!output) {
        emit grabber->failed(WaylandSurfaceGrabber::RendererNotReady);
        return;
    }

    // We cannot grab the surface now, we need to have a current opengl context, so we
    // need to be in the render thread
    class GrabState : public QRunnable
    {
    public:
        WaylandSurfaceGrabber *grabber = nullptr;
        WaylandBufferRef buffer;

        void run() override
        {
            QOpenGLFramebufferObject fbo(buffer.size());
            fbo.bind();
            QOpenGLTextureBlitter blitter;
            blitter.create();

            glViewport(0, 0, buffer.size().width(), buffer.size().height());

            QOpenGLTextureBlitter::Origin surfaceOrigin =
                buffer.origin() == WaylandSurface::OriginTopLeft
                ? QOpenGLTextureBlitter::OriginTopLeft
                : QOpenGLTextureBlitter::OriginBottomLeft;

            auto texture = buffer.toOpenGLTexture();
            blitter.bind(texture->target());
            blitter.blit(texture->textureId(), QMatrix4x4(), surfaceOrigin);
            blitter.release();

            emit grabber->success(fbo.toImage());
        }
    };

    GrabState *state = new GrabState;
    state->grabber = grabber;
    state->buffer = buffer;
    static_cast<QQuickWindow *>(output->window())->scheduleRenderJob(state, QQuickWindow::AfterRenderingStage);
#else
    emit grabber->failed(WaylandSurfaceGrabber::UnknownBufferType);
#endif
}

} // namespace Compositor

} // namespace Aurora
