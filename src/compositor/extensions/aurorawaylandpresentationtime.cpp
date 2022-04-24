/****************************************************************************
**
** Copyright (C) 2021 LG Electronics Inc.
** Contact: http://www.qt.io/licensing/
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

#include "aurorawaylandpresentationtime_p.h"
#include "aurorawaylandpresentationtime_p_p.h"

#include <time.h>
#include <QQuickWindow>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#  include <QScreen>
#endif
#include <LiriAuroraCompositor/WaylandView>
#include <LiriAuroraCompositor/WaylandQuickItem>

namespace Aurora {

namespace Compositor {

/*!
 * \qmltype PresentationTime
 * \instantiates WaylandPresentationTime
 * \inqmlmodule Aurora.Compositor.PresentationTime
 * \since 6.3
 * \brief Provides tracking the timing when a frame is presented on screen.
 *
 * The PresentationTime extension provides a way to track rendering timing
 * for a surface. Client can request feedbacks associated with a surface,
 * then compositor send events for the feedback with the time when the surface
 * is presented on-screen.
 *
 * PresentationTime corresponds to the Wayland \c wp_presentation interface.
 *
 * To provide the functionality of the presentationtime extension in a compositor, create
 * an instance of the PresentationTime component and add it to the list of extensions
 * supported by the compositor:
 *
 * Then, call sendFeedback() when a surface is presented on screen.
 * Usually, the timing can be obtained from drm page flip event.
 *
 * \qml
 * import Aurora.Compositor.PresentationTime
 *
 * WaylandCompositor {
 *     PresentationTime {
 *         id: presentationTime
 *     }
 * }
 * \endqml
 */

/*!
 * \class WaylandPresentationTime
 * \inmodule AuroraCompositor
 * \since 6.3
 * \brief The WaylandPresentationTime class is an extension to get timing for on-screen presentation.
 *
 * The WaylandPresentationTime extension provides a way to track rendering timing
 * for a surface. Client can request feedbacks associated with a surface,
 * then compositor send events for the feedback with the time when the surface
 * is presented on-screen.
 *
 * WaylandPresentationTime corresponds to the Wayland \c wp_presentation interface.
 */


/*!
 * Constructs a WaylandPresentationTime object for \a compositor.
 */
WaylandPresentationTime::WaylandPresentationTime(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate(compositor)
    , d_ptr(new WaylandPresentationTimePrivate(this))
{

}

WaylandPresentationTime::~WaylandPresentationTime()
{
}

/*!
 * Constructs an empty WaylandPresentationTime object.
 */
WaylandPresentationTime::WaylandPresentationTime()
    : WaylandCompositorExtensionTemplate()
    , d_ptr(new WaylandPresentationTimePrivate(this))
{
}

/*!
 * Initializes the extension.
 */
void WaylandPresentationTime::initialize()
{
    Q_D(WaylandPresentationTime);

    if (isInitialized()) {
        qWarning() << "WaylandPresentationTime is already initialized";
        return;
    }

    WaylandCompositor *compositor = this->compositor();
    if (compositor == nullptr) {
        qWarning() << "Failed to find WaylandCompositor when initializing WaylandPresentationTime";
        return;
    }

    WaylandCompositorExtensionTemplate::initialize();

    d->init(compositor->display(), /* version */ 1);
}

WaylandCompositor *WaylandPresentationTime::compositor() const
{
    return qobject_cast<WaylandCompositor *>(extensionContainer());
}

/*!
 * \qmlmethod void WaylandCompositor::PresentationTime::sendFeedback(Window window, int sequence, int sec, int nsec)
 *
 * Interface to notify that a frame is presented on screen using \a window.
 * If your platform supports DRM events, \c page_flip_handler is the proper timing to send it.
 * The \a sequence is the refresh counter. \a sec and \a nsec hold the
 * seconds and nanoseconds parts of the presentation timestamp, respectively.
 */

/*!
 * Interface to notify that a frame is presented on screen using \a window.
 * If your platform supports DRM events, \c page_flip_handler is the proper timing to send it.
 * The \a sequence is the refresh counter. \a tv_sec and \a tv_nsec hold the
 * seconds and nanoseconds parts of the presentation timestamp, respectively.
 */
void WaylandPresentationTime::sendFeedback(QQuickWindow *window, quint64 sequence, quint64 tv_sec, quint32 tv_nsec)
{
    if (!window)
        return;

    quint32 refresh_nsec = window->screen()->refreshRate() != 0 ? 1000000000 / window->screen()->refreshRate() : 0;

    emit presented(sequence, tv_sec, tv_nsec, refresh_nsec);
}

/*!
 * Returns the Wayland interface for the WaylandPresentationTime.
 */
const struct wl_interface *WaylandPresentationTime::interface()
{
    return WaylandPresentationTimePrivate::interface();
}

/*!
 * \internal
 */
QByteArray WaylandPresentationTime::interfaceName()
{
    return WaylandPresentationTimePrivate::interfaceName();
}

PresentationFeedback::PresentationFeedback(WaylandPresentationTime *pTime, WaylandSurface *surface, struct ::wl_client *client, uint32_t id, int version)
    : wp_presentation_feedback(client, id, version)
    , m_presentationTime(pTime)
{
    setSurface(surface);
}

void PresentationFeedback::setSurface(WaylandSurface *qwls)
{
    if (!qwls) {
        discard();
        return;
    }

    m_surface = qwls;

    connect(qwls, &WaylandSurface::damaged, this, &PresentationFeedback::onSurfaceCommit);
    connect(qwls, &WaylandSurface::destroyed, this, &PresentationFeedback::discard);

    WaylandView *view = qwls ? qwls->primaryView() : nullptr;
    //The surface has not committed yet.
    if (!view) {
        connect(qwls, &WaylandSurface::hasContentChanged, this, &PresentationFeedback::onSurfaceMapped);
        return;
    }

    maybeConnectToWindow(view);
}

void PresentationFeedback::onSurfaceCommit()
{
    // There is a new commit before sync so that discard this feedback.
    if (m_committed) {
        discard();
        return;
    }
    m_committed = true;
}

void PresentationFeedback::onSurfaceMapped()
{
    WaylandView *view = m_surface->primaryView();
    if (!view) {
        qWarning() << "The mapped surface has no view";
        discard();
        return;
    }

    maybeConnectToWindow(view);
}

void PresentationFeedback::maybeConnectToWindow(WaylandView *view)
{
    WaylandQuickItem *item = view ? qobject_cast<WaylandQuickItem *>(view->renderObject()) : nullptr;
    if (!item) {
        qWarning() << "WaylandPresentationTime only works with QtQuick compositors" << view;
        discard();
        return;
    }

    connect(item, &QQuickItem::windowChanged, this, &PresentationFeedback::onWindowChanged);
    // wait for having window
    if (!item->window()) {
       return;
    }

    connectToWindow(item->window());
}

void PresentationFeedback::onWindowChanged()
{
    WaylandQuickItem *item = qobject_cast<WaylandQuickItem *>(sender());
    QQuickWindow *window = item ? item->window() : nullptr;

    if (!window) {
        qWarning() << "WaylandPresentationTime only works with QtQuick compositors" << item;
        discard();
        /* Actually, the commit is not discarded yet. If the related item has new window,
           the commit can be presented on screen. So we can choose not to discard the feedback
           until item has new window or the surface is destroyed. */
        return;
    }

    // Check if the connected window is changed
    if (m_connectedWindow && m_connectedWindow != window)
        m_connectedWindow->disconnect(this);

    connectToWindow(window);
}

void PresentationFeedback::connectToWindow(QQuickWindow *window)
{
    if (!window) {
        discard();
        return;
    }

    m_connectedWindow = window;

    connect(window, &QQuickWindow::beforeSynchronizing, this, &PresentationFeedback::onSync);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(window, &QQuickWindow::afterFrameEnd, this, &PresentationFeedback::onSwapped);
#else
    connect(window, &QQuickWindow::frameSwapped, this, &PresentationFeedback::onSwapped);
#endif
}

void PresentationFeedback::onSync()
{
    QQuickWindow *window = qobject_cast<QQuickWindow *>(sender());

    if (m_committed) {
        disconnect(m_surface, &WaylandSurface::damaged, this, &PresentationFeedback::onSurfaceCommit);
        disconnect(window, &QQuickWindow::beforeSynchronizing, this, &PresentationFeedback::onSync);
        m_sync = true;
    }
}

void PresentationFeedback::onSwapped()
{
    QQuickWindow *window = qobject_cast<QQuickWindow *>(sender());

    if (m_sync) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        disconnect(window, &QQuickWindow::afterFrameEnd, this, &PresentationFeedback::onSwapped);
#else
        disconnect(window, &QQuickWindow::frameSwapped, this, &PresentationFeedback::onSwapped);
#endif
        connect(m_presentationTime, &WaylandPresentationTime::presented, this, &PresentationFeedback::sendPresented);
    }
}

void PresentationFeedback::discard()
{
    send_discarded();
    destroy();
}

void PresentationFeedback::sendSyncOutput()
{
    WaylandCompositor *compositor = presentationTime()->compositor();
    if (!compositor) {
        qWarning() << "No compositor container to send sync_output";
        return;
    }

    WaylandView *view = surface()->primaryView();
    WaylandOutput *output = view ? view->output() : nullptr;
    struct ::wl_resource *r = output ? output->resourceForClient(WaylandClient::fromWlClient(compositor, resource()->client())) : nullptr;

    if (r)
        send_sync_output(r);
}

void PresentationFeedback::sendPresented(quint64 sequence, quint64 tv_sec, quint32 tv_nsec, quint32 refresh_nsec)
{
    sendSyncOutput();

    send_presented(tv_sec >> 32, tv_sec, tv_nsec, refresh_nsec, sequence >> 32, sequence,
            PrivateServer::wp_presentation_feedback::kind_vsync
            | PrivateServer::wp_presentation_feedback::kind_hw_clock
            | PrivateServer::wp_presentation_feedback::kind_hw_completion);

    destroy();
}

void PresentationFeedback::destroy()
{
    wl_resource_destroy(resource()->handle);
}

void PresentationFeedback::wp_presentation_feedback_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

WaylandPresentationTimePrivate::WaylandPresentationTimePrivate(WaylandPresentationTime *self)
    : WaylandCompositorExtensionPrivate(self)
{
}

void WaylandPresentationTimePrivate::wp_presentation_bind_resource(Resource *resource)
{
    send_clock_id(resource->handle, CLOCK_MONOTONIC);
}

void WaylandPresentationTimePrivate::wp_presentation_feedback(Resource *resource, struct ::wl_resource *surface, uint32_t callback)
{
    Q_Q(WaylandPresentationTime);

    WaylandSurface *qwls = WaylandSurface::fromResource(surface);
    if (!qwls)
        return;

    new PresentationFeedback(q, qwls, resource->client(), callback, /* version */ 1);
}

} // namespace Compositor

} // namespace Aurora
