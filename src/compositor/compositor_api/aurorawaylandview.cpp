/****************************************************************************
**
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

#include "aurorawaylandview.h"
#include "aurorawaylandview_p.h"
#include "aurorawaylandsurface.h"
#include <LiriAuroraCompositor/WaylandSeat>
#include <LiriAuroraCompositor/WaylandCompositor>

#include <LiriAuroraCompositor/private/aurorawaylandsurface_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandoutput_p.h>

#include <QtCore/QMutex>

namespace Aurora {

namespace Compositor {

void WaylandViewPrivate::markSurfaceAsDestroyed(WaylandSurface *surface)
{
    Q_Q(WaylandView);
    Q_ASSERT(surface == this->surface);

    setSurface(nullptr);
    QPointer<WaylandView> deleteGuard(q);
    emit q->surfaceDestroyed();
    if (!deleteGuard.isNull())
        clearFrontBuffer();
}

/*!
 * \qmltype WaylandView
 * \instantiates WaylandView
 * \inqmlmodule Aurora.Compositor
 * \since 5.8
 * \brief Represents a view of a surface on an output.
 *
 * The WaylandView corresponds to the presentation of a surface on a specific
 * output, managing the buffers that contain the contents to be rendered.
 * You can have several views into the same surface.
 */

/*!
 * \class WaylandView
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The WaylandView class represents a view of a surface on an output.
 *
 * The WaylandView corresponds to the presentation of a surface on a specific
 * output, managing the buffers that contain the contents to be rendered.
 * You can have several views into the same surface.
 */

/*!
 * Constructs a WaylandView with the given \a renderObject and \a parent.
 */
WaylandView::WaylandView(QObject *renderObject, QObject *parent)
    : QObject(*new WaylandViewPrivate(),parent)
{
    d_func()->renderObject = renderObject;
}

/*!
 * Destroys the WaylandView.
 */
WaylandView::~WaylandView()
{
    Q_D(WaylandView);
    if (d->surface) {
        if (d->output)
            WaylandOutputPrivate::get(d->output)->removeView(this, d->surface);

        WaylandSurfacePrivate::get(d->surface)->derefView(this);
    }

}

/*!
* \internal
*  Didn't we decide to remove this property?
*/
QObject *WaylandView::renderObject() const
{
    Q_D(const WaylandView);
    return d->renderObject;
}

/*!
 * \qmlproperty WaylandSurface QtWaylandCompositor::WaylandView::surface
 *
 * This property holds the surface viewed by this WaylandView.
 */

/*!
 * \property WaylandView::surface
 *
 * This property holds the surface viewed by this WaylandView.
 */
WaylandSurface *WaylandView::surface() const
{
    Q_D(const WaylandView);
    return d->surface;
}


void WaylandViewPrivate::setSurface(WaylandSurface *newSurface)
{
    Q_Q(WaylandView);
    if (surface) {
        WaylandSurfacePrivate::get(surface)->derefView(q);
        if (output)
            WaylandOutputPrivate::get(output)->removeView(q, surface);
    }

    surface = newSurface;

    nextBuffer = WaylandBufferRef();
    nextBufferCommitted = false;
    nextDamage = QRegion();

    if (surface) {
        WaylandSurfacePrivate::get(surface)->refView(q);
        if (output)
            WaylandOutputPrivate::get(output)->addView(q, surface);
    }
}

void WaylandViewPrivate::clearFrontBuffer()
{
    if (!bufferLocked) {
        currentBuffer = WaylandBufferRef();
        currentDamage = QRegion();
    }
}

void WaylandView::setSurface(WaylandSurface *newSurface)
{
    Q_D(WaylandView);
    if (d->surface == newSurface)
        return;

    d->setSurface(newSurface);
    d->clearFrontBuffer();
    emit surfaceChanged();
}

/*!
 * \qmlproperty WaylandOutput QtWaylandCompositor::WaylandView::output
 *
 * This property holds the output on which this view displays its surface.
 */

/*!
 * \property WaylandView::output
 *
 * This property holds the output on which this view displays its surface.
 */
WaylandOutput *WaylandView::output() const
{
    Q_D(const WaylandView);
    return d->output;
}

void WaylandView::setOutput(WaylandOutput *newOutput)
{
    Q_D(WaylandView);
    if (d->output == newOutput)
        return;

    if (d->output && d->surface)
        WaylandOutputPrivate::get(d->output)->removeView(this, d->surface);

    d->output = newOutput;

    if (d->output && d->surface)
        WaylandOutputPrivate::get(d->output)->addView(this, d->surface);

    emit outputChanged();
}

/*!
 * This function is called when a new \a buffer is committed to this view's surface.
 * \a damage contains the region that is different from the current buffer, i.e. the
 * region that needs to be updated.
 * The new \a buffer will become current on the next call to advance().
 *
 * Subclasses that reimplement this function \e must call the base implementation.
 */
void WaylandView::bufferCommitted(const WaylandBufferRef &buffer, const QRegion &damage)
{
    Q_D(WaylandView);
    QMutexLocker locker(&d->bufferMutex);
    d->nextBuffer = buffer;
    d->nextDamage = damage;
    d->nextBufferCommitted = true;
}

/*!
 * Updates the current buffer and damage region to the latest version committed by the client.
 * Returns true if new content was committed since the previous call to advance().
 * Otherwise returns false.
 *
 * \sa currentBuffer(), currentDamage()
 */
bool WaylandView::advance()
{
    Q_D(WaylandView);

    if (!d->nextBufferCommitted && !d->forceAdvanceSucceed)
        return false;

    if (d->bufferLocked)
        return false;

    if (d->surface && d->surface->primaryView() == this) {
        const auto views = d->surface->views();
        for (WaylandView *view : views) {
            if (view != this && view->allowDiscardFrontBuffer() && view->d_func()->currentBuffer == d->currentBuffer)
                view->discardCurrentBuffer();
        }
    }

    QMutexLocker locker(&d->bufferMutex);
    d->forceAdvanceSucceed = false;
    d->nextBufferCommitted = false;
    d->currentBuffer = d->nextBuffer;
    d->currentDamage = d->nextDamage;
    return true;
}

/*!
 * Force the view to discard its current buffer, to allow it to be reused on the client side.
 */
void WaylandView::discardCurrentBuffer()
{
    Q_D(WaylandView);
    QMutexLocker locker(&d->bufferMutex);
    d->currentBuffer = WaylandBufferRef();
    d->forceAdvanceSucceed = true;
}

/*!
 * Returns a reference to this view's current buffer.
 */
WaylandBufferRef WaylandView::currentBuffer()
{
    Q_D(WaylandView);
    QMutexLocker locker(&d->bufferMutex);
    return d->currentBuffer;
}

/*!
 * Returns the current damage region of this view.
 */
QRegion WaylandView::currentDamage()
{
    Q_D(WaylandView);
    QMutexLocker locker(&d->bufferMutex);
    return d->currentDamage;
}

/*!
 * \qmlproperty bool QtWaylandCompositor::WaylandView::bufferLocked
 *
 * This property holds whether the view's buffer is currently locked. When
 * the buffer is locked, advance() will not advance to the next buffer and
 * returns \c false.
 *
 * The default is \c false.
 */

/*!
 * \property WaylandView::bufferLocked
 *
 * This property holds whether the view's buffer is currently locked. When
 * the buffer is locked, advance() will not advance to the next buffer
 * and returns \c false.
 *
 * The default is \c false.
 */
bool WaylandView::isBufferLocked() const
{
    Q_D(const WaylandView);
    return d->bufferLocked;
}

void WaylandView::setBufferLocked(bool locked)
{
    Q_D(WaylandView);
    if (d->bufferLocked == locked)
        return;
    d->bufferLocked = locked;
    emit bufferLockedChanged();
}
/*!
 * \qmlproperty bool QtWaylandCompositor::WaylandView::allowDiscardFrontBuffer
 *
 * By default, the view locks the current buffer until advance() is called. Set this property
 * to true to allow Qt to release the buffer when the primary view is no longer using it.
 *
 * This can be used to avoid the situation where a secondary view that updates on a lower
 * frequency will throttle the frame rate of the client application.
 */

/*!
 * \property WaylandView::allowDiscardFrontBuffer
 *
 * By default, the view locks the current buffer until advance() is called. Set this property
 * to \c true to allow Qt to release the buffer when the primary view is no longer using it.
 *
 * This can be used to avoid the situation where a secondary view that updates on a lower
 * frequency will throttle the frame rate of the client application.
 */
bool WaylandView::allowDiscardFrontBuffer() const
{
    Q_D(const WaylandView);
    return d->allowDiscardFrontBuffer;
}

void WaylandView::setAllowDiscardFrontBuffer(bool discard)
{
    Q_D(WaylandView);
    if (d->allowDiscardFrontBuffer == discard)
        return;
    d->allowDiscardFrontBuffer = discard;
    emit allowDiscardFrontBufferChanged();
}

/*!
 * Makes this WaylandView the primary view for the surface.
 *
 * It has no effect if this WaylandView is not holding any WaylandSurface
 *
 * \sa WaylandSurface::primaryView
 */
void WaylandView::setPrimary()
{
    Q_D(WaylandView);
    if (d->surface)
        d->surface->setPrimaryView(this);
    else
        qWarning("Calling setPrimary() on a WaylandView without a surface has no effect.");
}

/*!
 * Returns true if this WaylandView is the primary view for the WaylandSurface
 *
 * \sa WaylandSurface::primaryView
 */
bool WaylandView::isPrimary() const
{
    Q_D(const WaylandView);
    return d->surface && d->surface->primaryView() == this;
}

/*!
 * Returns the Wayland surface resource for this WaylandView.
 */
struct wl_resource *WaylandView::surfaceResource() const
{
    Q_D(const WaylandView);
    if (!d->surface)
        return nullptr;
    return d->surface->resource();
}

} // namespace Compositor

} // namespace Aurora
