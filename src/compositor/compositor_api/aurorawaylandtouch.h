// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDTOUCH_H
#define AURORA_COMPOSITOR_WAYLANDTOUCH_H

#include <LiriAuroraCompositor/WaylandCompositorExtension>

#include <QtCore/QObject>
#include <QtGui/QTouchEvent>

struct wl_resource;

namespace Aurora {

namespace Compositor {

class WaylandTouch;
class WaylandTouchPrivate;
class WaylandSeat;
class WaylandView;
class WaylandClient;
class WaylandSurface;

class LIRIAURORACOMPOSITOR_EXPORT WaylandTouch : public WaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandTouch)
public:
    WaylandTouch(WaylandSeat *seat, QObject *parent = nullptr);
    ~WaylandTouch();

    WaylandSeat *seat() const;
    WaylandCompositor *compositor() const;

    virtual uint sendTouchPointEvent(WaylandSurface *surface, int id, const QPointF &position, Qt::TouchPointState state);
    virtual void sendFrameEvent(WaylandClient *client);
    virtual void sendCancelEvent(WaylandClient *client);
    virtual void sendFullTouchEvent(WaylandSurface *surface, QTouchEvent *event);

    virtual void addClient(WaylandClient *client, uint32_t id, uint32_t version);

private:
    QScopedPointer<WaylandTouchPrivate> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

#endif  /*QWAYLANDTOUCH_H*/
