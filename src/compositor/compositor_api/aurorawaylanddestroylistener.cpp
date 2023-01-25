// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylanddestroylistener.h"
#include "aurorawaylanddestroylistener_p.h"

namespace Aurora {

namespace Compositor {

WaylandDestroyListenerPrivate::WaylandDestroyListenerPrivate(WaylandDestroyListener *self)
    : q_ptr(self)
{
    listener.parent = this;
    listener.listener.notify = handler;
    wl_list_init(&listener.listener.link);
}

WaylandDestroyListener::WaylandDestroyListener(QObject *parent)
    : QObject(parent)
    , d_ptr(new WaylandDestroyListenerPrivate(this))
{
}

WaylandDestroyListener::~WaylandDestroyListener()
{
}

void WaylandDestroyListener::listenForDestruction(::wl_resource *resource)
{
    Q_D(WaylandDestroyListener);
    wl_resource_add_destroy_listener(resource, &d->listener.listener);
}

void WaylandDestroyListener::reset()
{
    Q_D(WaylandDestroyListener);
    wl_list_remove(&d->listener.listener.link);
    wl_list_init(&d->listener.listener.link);
}

void WaylandDestroyListenerPrivate::handler(wl_listener *listener, void *data)
{
    WaylandDestroyListenerPrivate *that = reinterpret_cast<Listener *>(listener)->parent;
    emit that->q_func()->fired(data);
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylanddestroylistener.cpp"
