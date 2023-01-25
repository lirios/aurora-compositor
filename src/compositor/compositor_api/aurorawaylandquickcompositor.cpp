// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
#include "aurorawaylandseat.h"
#include "aurorawaylandsurfacegrabber.h"

namespace Aurora {

namespace Compositor {

class WaylandQuickCompositorPrivate : public WaylandCompositorPrivate
{
public:
    explicit WaylandQuickCompositorPrivate(WaylandQuickCompositor *self)
        : WaylandCompositorPrivate(self)
        , q_ptr(self)
        , m_viewporter(new WaylandViewporter(self))
    {
    }
protected:
    WaylandSurface *createDefaultSurface() override
    {
        return new WaylandQuickSurface();
    }
private:
    WaylandQuickCompositor *q_ptr = nullptr;
    QScopedPointer<WaylandViewporter> m_viewporter;
};

WaylandQuickCompositor::WaylandQuickCompositor(QObject *parent)
    : WaylandCompositor(parent)
    , d_ptr(new WaylandQuickCompositorPrivate(this))
{
}

WaylandQuickCompositor::~WaylandQuickCompositor()
{
}

/*!
 * \qmlproperty list AuroraCompositor::WaylandCompositor::extensions
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

QQmlListProperty<WaylandOutput> WaylandQuickCompositor::outputsListProperty()
{
    return QQmlListProperty<WaylandOutput>(this, this,
                                           &WaylandQuickCompositor::outputsCountFunction,
                                           &WaylandQuickCompositor::outputsAtFunction);
}

QQmlListProperty<WaylandSeat> WaylandQuickCompositor::seatsListProperty()
{
    return QQmlListProperty<WaylandSeat>(this, this,
                                         &WaylandQuickCompositor::seatsCountFunction,
                                         &WaylandQuickCompositor::seatsAtFunction);
}

int WaylandQuickCompositor::outputsCountFunction(QQmlListProperty<WaylandOutput> *list)
{
    return static_cast<WaylandCompositor *>(list->data)->outputs().size();
}

WaylandOutput *WaylandQuickCompositor::outputsAtFunction(QQmlListProperty<WaylandOutput> *list, int index)
{
    return static_cast<WaylandCompositor *>(list->data)->outputs().at(index);
}

int WaylandQuickCompositor::seatsCountFunction(QQmlListProperty<WaylandSeat> *list)
{
    return static_cast<WaylandCompositor *>(list->data)->seats().size();
}

WaylandSeat *WaylandQuickCompositor::seatsAtFunction(QQmlListProperty<WaylandSeat> *list, int index)
{
    return static_cast<WaylandCompositor *>(list->data)->seats().at(index);
}

} // namespace Compositor

} // namespace Aurora
