/****************************************************************************
**
** Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "aurorawaylandclient.h"
#include <QtCore/private/qobject_p.h>

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/private/aurorawaylandcompositor_p.h>


#include <wayland-server-core.h>
#include <wayland-util.h>

namespace Aurora {

namespace Compositor {

class WaylandClientPrivate : public QObjectPrivate
{
public:
    WaylandClientPrivate(WaylandCompositor *compositor, wl_client *_client)
        : compositor(compositor)
        , client(_client)
    {
        // Save client credentials
        wl_client_get_credentials(client, &pid, &uid, &gid);
    }

    ~WaylandClientPrivate() override
    {
    }

    static void client_destroy_callback(wl_listener *listener, void *data)
    {
        Q_UNUSED(data);

        WaylandClient *client = reinterpret_cast<Listener *>(listener)->parent;
        Q_ASSERT(client != nullptr);
        delete client;
    }

    WaylandCompositor *compositor = nullptr;
    wl_client *client = nullptr;

    uid_t uid;
    gid_t gid;
    pid_t pid;

    struct Listener {
        wl_listener listener;
        WaylandClient *parent = nullptr;
    };
    Listener listener;

    WaylandClient::TextInputProtocols mTextInputProtocols = WaylandClient::NoProtocol;
};

/*!
 * \qmltype WaylandClient
 * \instantiates WaylandClient
 * \inqmlmodule Aurora.Compositor
 * \since 5.8
 * \brief Represents a client connecting to the WaylandCompositor.
 *
 * This type represents a client connecting to the compositor using the Wayland protocol.
 * It corresponds to the Wayland interface wl_client.
 */

/*!
 * \class WaylandClient
 * \inmodule AuroraCompositor
 * \since 5.8
 * \brief The WaylandClient class represents a client connecting to the WaylandCompositor.
 *
 * This class corresponds to a client connecting to the compositor using the Wayland protocol.
 * It corresponds to the Wayland interface wl_client.
 */

/*!
 * Constructs a WaylandClient for the \a compositor and the Wayland \a client.
 */
WaylandClient::WaylandClient(WaylandCompositor *compositor, wl_client *client)
    : QObject(*new WaylandClientPrivate(compositor, client))
{
    Q_D(WaylandClient);

    // Destroy wrapper when the client goes away
    d->listener.parent = this;
    d->listener.listener.notify = WaylandClientPrivate::client_destroy_callback;
    wl_client_add_destroy_listener(client, &d->listener.listener);

    WaylandCompositorPrivate::get(compositor)->addClient(this);
}

/*!
 * Destroys the WaylandClient.
 */
WaylandClient::~WaylandClient()
{
    Q_D(WaylandClient);

    // Remove listener from signal
    wl_list_remove(&d->listener.listener.link);

    WaylandCompositorPrivate::get(d->compositor)->removeClient(this);
}

/*!
 * Returns the WaylandClient corresponding to the Wayland client \a wlClient and \a compositor.
 * If a WaylandClient has not already been created for a client, it is
 * created and returned.
 */
WaylandClient *WaylandClient::fromWlClient(WaylandCompositor *compositor, wl_client *wlClient)
{
    if (!wlClient)
        return nullptr;

    WaylandClient *client = nullptr;

    wl_listener *l = wl_client_get_destroy_listener(wlClient,
        WaylandClientPrivate::client_destroy_callback);
    if (l)
        client = reinterpret_cast<WaylandClientPrivate::Listener *>(
            wl_container_of(l, (WaylandClientPrivate::Listener *)nullptr, listener))->parent;

    if (!client) {
        // The original idea was to create WaylandClient instances when
        // a client bound wl_compositor, but it's legal for a client to
        // bind several times resulting in multiple WaylandClient
        // instances for the same wl_client therefore we create it from
        // here on demand
        client = new WaylandClient(compositor, wlClient);
    }

    return client;
}

/*!
 * \qmlproperty WaylandCompositor AuroraCompositor::WaylandClient::compositor
 *
 * This property holds the compositor of this WaylandClient.
 */

/*!
 * \property WaylandClient::compositor
 *
 * This property holds the compositor of this WaylandClient.
 */
WaylandCompositor *WaylandClient::compositor() const
{
    Q_D(const WaylandClient);

    return d->compositor;
}

/*!
 * Returns the Wayland client of this WaylandClient.
 */
wl_client *WaylandClient::client() const
{
    Q_D(const WaylandClient);

    return d->client;
}

/*!
 * \qmlproperty int AuroraCompositor::WaylandClient::userId
 *
 * This property holds the user id of this WaylandClient.
 */

/*!
 * \property WaylandClient::userId
 * \readonly
 *
 * This property holds the user id of this WaylandClient.
 */
qint64 WaylandClient::userId() const
{
    Q_D(const WaylandClient);

    return d->uid;
}

/*!
 * \qmlproperty int AuroraCompositor::WaylandClient::groupId
 * \readonly
 *
 * This property holds the group id of this WaylandClient.
 */

/*!
 * \property WaylandClient::groupId
 *
 * This property holds the group id of this WaylandClient.
 */
qint64 WaylandClient::groupId() const
{
    Q_D(const WaylandClient);

    return d->gid;
}

/*!
 * \qmlproperty int AuroraCompositor::WaylandClient::processId
 * \readonly
 *
 * This property holds the process id of this WaylandClient.
 */

/*!
 * \property WaylandClient::processId
 *
 * This property holds the process id of this WaylandClient.
 */
qint64 WaylandClient::processId() const
{
    Q_D(const WaylandClient);

    return d->pid;
}

/*!
 * \qmlmethod void AuroraCompositor::WaylandClient::kill(signal)
 *
 * Kills the client with the specified \a signal.
 */

/*!
 * Kills the client with the specified \a signal.
 */
void WaylandClient::kill(int signal)
{
    Q_D(WaylandClient);

    ::kill(d->pid, signal);
}

/*!
 * \qmlmethod void AuroraCompositor::WaylandClient::close()
 *
 * Closes the client
 */

/*!
 * Closes the client.
 */
void WaylandClient::close()
{
    Q_D(WaylandClient);
    d->compositor->destroyClient(this);
}

WaylandClient::TextInputProtocols WaylandClient::textInputProtocols() const
{
    Q_D(const WaylandClient);
    return d->mTextInputProtocols;
}

void WaylandClient::setTextInputProtocols(TextInputProtocols p)
{
    Q_D(WaylandClient);
    if (d->mTextInputProtocols != p)
        d->mTextInputProtocols = p;
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandclient.cpp"

