/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "aurorawaylandtouch.h"
#include "aurorawaylandtouch_p.h"

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandSeat>
#include <LiriAuroraCompositor/WaylandView>
#include <LiriAuroraCompositor/WaylandClient>

#include <LiriAuroraCompositor/private/aurorawlqttouch_p.h>

namespace Aurora {

namespace Compositor {

WaylandTouchPrivate::WaylandTouchPrivate(WaylandTouch *touch, WaylandSeat *seat)
    : q_ptr(touch)
    , seat(seat)
{
}

void WaylandTouchPrivate::touch_release(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

uint WaylandTouchPrivate::sendDown(WaylandSurface *surface, uint32_t time, int touch_id, const QPointF &position)
{
    Q_Q(WaylandTouch);
    auto focusResource = resourceMap().value(surface->client()->client());
    if (!focusResource)
        return 0;

    uint32_t serial = q->compositor()->nextSerial();

    wl_touch_send_down(focusResource->handle, serial, time, surface->resource(), touch_id,
                       wl_fixed_from_double(position.x()), wl_fixed_from_double(position.y()));
    return serial;
}

uint WaylandTouchPrivate::sendUp(WaylandClient *client, uint32_t time, int touch_id)
{
    auto focusResource = resourceMap().value(client->client());

    if (!focusResource)
        return 0;

    uint32_t serial = compositor()->nextSerial();

    wl_touch_send_up(focusResource->handle, serial, time, touch_id);
    return serial;
}

void WaylandTouchPrivate::sendMotion(WaylandClient *client, uint32_t time, int touch_id, const QPointF &position)
{
    auto focusResource = resourceMap().value(client->client());

    if (!focusResource)
        return;

    wl_touch_send_motion(focusResource->handle, time, touch_id,
                         wl_fixed_from_double(position.x()), wl_fixed_from_double(position.y()));
}

int WaylandTouchPrivate::toSequentialWaylandId(int touchId)
{
    const int waylandId = ids.indexOf(touchId);
    if (waylandId != -1)
        return waylandId;
    const int availableId = ids.indexOf(-1);
    if (availableId != -1) {
        ids[availableId] = touchId;
        return availableId;
    }
    ids.append(touchId);
    return ids.length() - 1;
}

/*!
 * \class WaylandTouch
 * \inmodule AuroraCompositor
 * \since 5.8
 * \brief The WaylandTouch class provides access to a touch device.
 *
 * This class provides access to the touch device in a WaylandSeat. It corresponds to
 * the Wayland interface wl_touch.
 */

/*!
 * Constructs a WaylandTouch for the \a seat and with the given \a parent.
 */
WaylandTouch::WaylandTouch(WaylandSeat *seat, QObject *parent)
    : WaylandObject(parent)
    , d_ptr(new WaylandTouchPrivate(this, seat))
{
}

WaylandTouch::~WaylandTouch()
{
}

/*!
 * Returns the input device for this WaylandTouch.
 */
WaylandSeat *WaylandTouch::seat() const
{
    Q_D(const WaylandTouch);
    return d->seat;
}

/*!
 * Returns the compositor for this WaylandTouch.
 */
WaylandCompositor *WaylandTouch::compositor() const
{
    Q_D(const WaylandTouch);
    return d->compositor();
}

/*!
 * Sends a touch point event to the touch device of \a surface with the given \a id,
 * \a position, and \a state.
 *
 * Returns the serial of the down or up event if sent, otherwise 0.
 */
uint WaylandTouch::sendTouchPointEvent(WaylandSurface *surface, int id, const QPointF &position, Qt::TouchPointState state)
{
    Q_D(WaylandTouch);
    uint32_t time = compositor()->currentTimeMsecs();
    uint serial = 0;
    switch (state) {
    case Qt::TouchPointPressed:
        serial = d->sendDown(surface, time, id, position);
        break;
    case Qt::TouchPointMoved:
        d->sendMotion(surface->client(), time, id, position);
        break;
    case Qt::TouchPointReleased:
        serial = d->sendUp(surface->client(), time, id);
        break;
    case Qt::TouchPointStationary:
        // stationary points are not sent through wayland, the client must cache them
        break;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    case Qt::TouchPointUnknownState:
        // Ignored
        break;
#endif
    }

    return serial;
}

/*!
 * Sends a touch frame event to the touch device of a \a client. This indicates the end of a
 * contact point list.
 */
void WaylandTouch::sendFrameEvent(WaylandClient *client)
{
    Q_D(WaylandTouch);
    auto focusResource = d->resourceMap().value(client->client());
    if (focusResource)
        d->send_frame(focusResource->handle);
}

/*!
 * Sends a touch cancel event to the touch device of a \a client.
 */
void WaylandTouch::sendCancelEvent(WaylandClient *client)
{
    Q_D(WaylandTouch);
    auto focusResource = d->resourceMap().value(client->client());
    if (focusResource)
        d->send_cancel(focusResource->handle);
}

/*!
 * Sends all touch points in \a event to the specified \a surface,
 * followed by a touch frame event.
 *
 * \sa sendTouchPointEvent(), sendFrameEvent()
 */
void WaylandTouch::sendFullTouchEvent(WaylandSurface *surface, QTouchEvent *event)
{
    Q_D(WaylandTouch);
    if (event->type() == QEvent::TouchCancel) {
        sendCancelEvent(surface->client());
        return;
    }

    Internal::TouchExtensionGlobal *ext = Internal::TouchExtensionGlobal::findIn(d->compositor());
    if (ext && ext->postTouchEvent(event, surface))
        return;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QList<QTouchEvent::TouchPoint> points = event->points();
    if (points.isEmpty())
        return;

    const int pointCount = points.count();
    for (int i = 0; i < pointCount; ++i) {
        const QTouchEvent::TouchPoint &tp(points.at(i));
        // Convert the local pos in the compositor window to surface-relative.
        const int id = d->toSequentialWaylandId(tp.id());
        sendTouchPointEvent(surface, id, tp.position(), Qt::TouchPointState(tp.state()));
        if (tp.state() == QEventPoint::Released)
            d->ids[id] = -1;
    }
#else
    const QList<QTouchEvent::TouchPoint> points = event->touchPoints();
    if (points.isEmpty())
        return;

    const int pointCount = points.count();
    for (int i = 0; i < pointCount; ++i) {
        const QTouchEvent::TouchPoint &tp(points.at(i));
        // Convert the local pos in the compositor window to surface-relative.
        const int id = d->toSequentialWaylandId(tp.id());
        sendTouchPointEvent(surface, id, tp.pos(), tp.state());
        if (tp.state() == Qt::TouchPointReleased)
            d->ids[id] = -1;
    }
#endif
    sendFrameEvent(surface->client());
}

/*!
 * \internal
 */
void WaylandTouch::addClient(WaylandClient *client, uint32_t id, uint32_t version)
{
    Q_D(WaylandTouch);
    d->add(client->client(), id, qMin<uint32_t>(PrivateServer::wl_touch::interfaceVersion(), version));
}

} // namespace Compositor

} // namespace Aurora
