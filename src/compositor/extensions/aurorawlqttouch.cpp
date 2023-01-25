// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawlqttouch_p.h"
#include "aurorawaylandview.h"
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#  include <QPointingDevice>
#endif
#include <QTouchEvent>
#include <QWindow>

namespace Aurora {

namespace Compositor {

namespace Internal {

static const int maxRawPos = 24;

TouchExtensionGlobal::TouchExtensionGlobal(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate(compositor)
    , PrivateServer::qt_touch_extension(compositor->display(), 1)
    , m_compositor(compositor)
    , m_posData(maxRawPos * 2)
{
}

TouchExtensionGlobal::~TouchExtensionGlobal()
{
}

static inline int toFixed(qreal f)
{
    return int(f * 10000);
}

bool TouchExtensionGlobal::postTouchEvent(QTouchEvent *event, WaylandSurface *surface)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QList<QTouchEvent::TouchPoint> points = event->points();
    const int pointCount = points.size();
    if (!pointCount)
        return false;

    wl_client *surfaceClient = surface->client()->client();
    uint32_t time = m_compositor->currentTimeMsecs();
    const int rescount = m_resources.size();

    for (int res = 0; res < rescount; ++res) {
        Resource *target = m_resources.at(res);
        if (target->client() != surfaceClient)
            continue;

        // We will use no touch_frame type of event, to reduce the number of
        // events flowing through the wire. Instead, the number of points sent is
        // included in the touch point events.
        int sentPointCount = 0;
        for (int i = 0; i < pointCount; ++i) {
            if (points.at(i).state() != QEventPoint::Stationary)
                ++sentPointCount;
        }

        for (int i = 0; i < pointCount; ++i) {
            const QTouchEvent::TouchPoint &tp(points.at(i));
            // Stationary points are never sent. They are cached on client side.
            if (tp.state() == QEventPoint::Stationary)
                continue;

            uint32_t id = tp.id();
            uint32_t state = (tp.state() & 0xFFFF) | (sentPointCount << 16);
            uint32_t flags = (int(event->pointingDevice()->capabilities()) << 16);

            int x = toFixed(tp.position().x());
            int y = toFixed(tp.position().y());
            int nx = toFixed(tp.normalizedPosition().x());
            int ny = toFixed(tp.normalizedPosition().y());
            int w = toFixed(tp.ellipseDiameters().width());
            int h = toFixed(tp.ellipseDiameters().height());
            int vx = toFixed(tp.velocity().x());
            int vy = toFixed(tp.velocity().y());
            uint32_t pressure = uint32_t(tp.pressure() * 255);

            QByteArray rawData;
            send_touch(target->handle,
                       time, id, state,
                       x, y, nx, ny, w, h,
                       pressure, vx, vy,
                       flags, rawData);
        }

        return true;
    }

    return false;
#else
    const QList<QTouchEvent::TouchPoint> points = event->touchPoints();
    const int pointCount = points.count();
    if (!pointCount)
        return false;

    wl_client *surfaceClient = surface->client()->client();
    uint32_t time = m_compositor->currentTimeMsecs();
    const int rescount = m_resources.count();

    for (int res = 0; res < rescount; ++res) {
        Resource *target = m_resources.at(res);
        if (target->client() != surfaceClient)
            continue;

        // We will use no touch_frame type of event, to reduce the number of
        // events flowing through the wire. Instead, the number of points sent is
        // included in the touch point events.
        int sentPointCount = 0;
        for (int i = 0; i < pointCount; ++i) {
            if (points.at(i).state() != Qt::TouchPointStationary)
                ++sentPointCount;
        }

        for (int i = 0; i < pointCount; ++i) {
            const QTouchEvent::TouchPoint &tp(points.at(i));
            // Stationary points are never sent. They are cached on client side.
            if (tp.state() == Qt::TouchPointStationary)
                continue;

            uint32_t id = tp.id();
            uint32_t state = (tp.state() & 0xFFFF) | (sentPointCount << 16);
            uint32_t flags = (tp.flags() & 0xFFFF) | (int(event->device()->capabilities()) << 16);

            int x = toFixed(tp.pos().x());
            int y = toFixed(tp.pos().y());
            int nx = toFixed(tp.normalizedPos().x());
            int ny = toFixed(tp.normalizedPos().y());
            int w = toFixed(tp.ellipseDiameters().width());
            int h = toFixed(tp.ellipseDiameters().height());
            int vx = toFixed(tp.velocity().x());
            int vy = toFixed(tp.velocity().y());
            uint32_t pressure = uint32_t(tp.pressure() * 255);

            QByteArray rawData;
            QVector<QPointF> rawPosList = tp.rawScreenPositions();
            int rawPosCount = rawPosList.count();
            if (rawPosCount) {
                rawPosCount = qMin(maxRawPos, rawPosCount);
                QVector<float>::iterator iter = m_posData.begin();
                for (int rpi = 0; rpi < rawPosCount; ++rpi) {
                    const QPointF &rawPos(rawPosList.at(rpi));
                    // This will stay in screen coordinates for performance
                    // reasons, clients using this data will presumably know
                    // what they are doing.
                    *iter++ = static_cast<float>(rawPos.x());
                    *iter++ = static_cast<float>(rawPos.y());
                }
                rawData = QByteArray::fromRawData(reinterpret_cast<const char*>(m_posData.constData()), sizeof(float) * rawPosCount * 2);
            }

            send_touch(target->handle,
                       time, id, state,
                       x, y, nx, ny, w, h,
                       pressure, vx, vy,
                       flags, rawData);
        }

        return true;
    }

    return false;
#endif
}

void TouchExtensionGlobal::setBehviorFlags(BehaviorFlags flags)
{
    if (m_flags == flags)
        return;

    m_flags = flags;
    behaviorFlagsChanged();
}

void TouchExtensionGlobal::touch_extension_bind_resource(Resource *resource)
{
    m_resources.append(resource);
    send_configure(resource->handle, m_flags);
}

void TouchExtensionGlobal::touch_extension_destroy_resource(Resource *resource)
{
    m_resources.removeOne(resource);
}

}

} // namespace Compositor

} // namespace Aurora
