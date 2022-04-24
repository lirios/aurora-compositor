/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "aurorawaylandxdgdecorationv1_p.h"

#include <LiriAuroraCompositor/WaylandXdgToplevel>
#include <LiriAuroraCompositor/private/aurorawaylandxdgshell_p.h>

#include <LiriAuroraCompositor/WaylandCompositor>
#include <QtCore/QObject>

namespace Aurora {

namespace Compositor {

/*!
    \qmltype XdgDecorationManagerV1
    \instantiates WaylandXdgDecorationManagerV1
    \inqmlmodule Aurora.Compositor.XdgShell
    \since 5.12
    \brief Provides an extension for negotiation of server-side and client-side window decorations.

    The XdgDecorationManagerV1 extension provides a way for a compositor to announce support for
    server-side window decorations, and for xdg-shell clients to communicate whether they prefer
    client-side or server-side decorations.

    XdgDecorationManagerV1 corresponds to the Wayland interface, \c zxdg_decoration_manager_v1.

    To provide the functionality of the extension in a compositor, create an instance of the
    XdgDecorationManagerV1 component and add it to the list of extensions supported by the compositor:

    \qml
    import Aurora.Compositor

    WaylandCompositor {
        // Xdg decoration manager assumes xdg-shell is being used
        XdgShell {
            onToplevelCreated: // ...
        }
        XdgDecorationManagerV1 {
            // Provide a hint to clients that support the extension they should use server-side
            // decorations.
            preferredMode: XdgToplevel.ServerSideDecoration
        }
    }
    \endqml

    \sa WaylandXdgToplevel::decorationMode,
        {Qt Wayland Compositor Examples - Server Side Decoration Compositor}
*/

/*!
    \class WaylandXdgDecorationManagerV1
    \inmodule AuroraCompositor
    \since 5.12
    \brief Provides an extension for negotiation of server-side and client-side window decorations.

    The WaylandXdgDecorationManagerV1 extension provides a way for a compositor to announce support
    for server-side window decorations, and for xdg-shell clients to communicate whether they prefer
    client-side or server-side decorations.

    WaylandXdgDecorationManagerV1 corresponds to the Wayland interface, \c zxdg_decoration_manager_v1.

    \sa WaylandXdgToplevel::decorationMode
*/

/*!
    Constructs a WaylandXdgDecorationManagerV1 object.
*/
WaylandXdgDecorationManagerV1::WaylandXdgDecorationManagerV1()
    : WaylandCompositorExtensionTemplate<WaylandXdgDecorationManagerV1>()
    , d_ptr(new WaylandXdgDecorationManagerV1Private(this))
{
}

WaylandXdgDecorationManagerV1::~WaylandXdgDecorationManagerV1()
{
}

/*!
    Initializes the extension.
*/
void WaylandXdgDecorationManagerV1::initialize()
{
    Q_D(WaylandXdgDecorationManagerV1);

    WaylandCompositorExtensionTemplate::initialize();
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find WaylandCompositor when initializing WaylandXdgDecorationV1";
        return;
    }
    d->init(compositor->display(), 1);
}

/*!
    \qmlproperty string AuroraCompositor::XdgDecorationManagerV1::preferredMode

    This property holds the decoration mode the compositor prefers.

    This is the mode used for clients that don't indicate a preference for server-side or
    client-side decorations.
*/
/*!
    \property WaylandXdgDecorationManagerV1::preferredMode

    This property holds the decoration mode the compositor prefers.

    This is the mode used for clients that don't indicate a preference for server-side or
    client-side decorations.
*/
WaylandXdgToplevel::DecorationMode WaylandXdgDecorationManagerV1::preferredMode() const
{
    Q_D(const WaylandXdgDecorationManagerV1);
    return d->m_preferredMode;
}

void WaylandXdgDecorationManagerV1::setPreferredMode(WaylandXdgToplevel::DecorationMode preferredMode)
{
    Q_D(WaylandXdgDecorationManagerV1);
    if (d->m_preferredMode == preferredMode)
        return;

    d->m_preferredMode = preferredMode;
    emit preferredModeChanged();
}

/*!
    Returns the Wayland interface for the WaylandXdgDecorationManagerV1.
*/
const wl_interface *WaylandXdgDecorationManagerV1::interface()
{
    return WaylandXdgDecorationManagerV1Private::interface();
}

WaylandXdgDecorationManagerV1Private::WaylandXdgDecorationManagerV1Private(WaylandXdgDecorationManagerV1 *self)
    : WaylandCompositorExtensionPrivate(self)
{
}

void WaylandXdgDecorationManagerV1Private::zxdg_decoration_manager_v1_get_toplevel_decoration(
        Resource *resource, uint id, wl_resource *toplevelResource)
{
    Q_Q(WaylandXdgDecorationManagerV1);

    auto *toplevel = WaylandXdgToplevel::fromResource(toplevelResource);
    if (!toplevel) {
        qWarning() << "Couldn't find toplevel for decoration";
        return;
    }

    //TODO: verify that the xdg surface is unconfigured, and post protocol error/warning

    auto *toplevelPrivate = WaylandXdgToplevelPrivate::get(toplevel);

    if (toplevelPrivate->m_decoration) {
        qWarning() << "zxdg_decoration_manager_v1.get_toplevel_decoration:"
                   << toplevel << "already has a decoration object, ignoring";
        //TODO: protocol error as well?
        return;
    }

    new WaylandXdgToplevelDecorationV1(toplevel, q, resource->client(), id);
}

WaylandXdgToplevelDecorationV1::WaylandXdgToplevelDecorationV1(WaylandXdgToplevel *toplevel,
                                                                 WaylandXdgDecorationManagerV1 *manager,
                                                                 wl_client *client, int id)
    : PrivateServer::zxdg_toplevel_decoration_v1(client, id, /*version*/ 1)
    , m_toplevel(toplevel)
    , m_manager(manager)
{
    Q_ASSERT(toplevel);
    auto *toplevelPrivate = WaylandXdgToplevelPrivate::get(toplevel);
    Q_ASSERT(!toplevelPrivate->m_decoration);
    toplevelPrivate->m_decoration = this;
    sendConfigure(manager->preferredMode());
}

WaylandXdgToplevelDecorationV1::~WaylandXdgToplevelDecorationV1()
{
    WaylandXdgToplevelPrivate::get(m_toplevel)->m_decoration = nullptr;
}

void WaylandXdgToplevelDecorationV1::sendConfigure(WaylandXdgToplevelDecorationV1::DecorationMode mode)
{
    if (configuredMode() == mode)
        return;

    switch (mode) {
    case DecorationMode::ClientSideDecoration:
        send_configure(mode_client_side);
        break;
    case DecorationMode::ServerSideDecoration:
        send_configure(mode_server_side);
        break;
    default:
        qWarning() << "Illegal mode in WaylandXdgToplevelDecorationV1::sendConfigure" << mode;
        break;
    }

    m_configuredMode = mode;
    emit m_toplevel->decorationModeChanged();
}

void WaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void WaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void WaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_set_mode(Resource *resource, uint32_t mode)
{
    Q_UNUSED(resource);
    m_clientPreferredMode = mode;
    handleClientPreferredModeChanged();
}

void WaylandXdgToplevelDecorationV1::zxdg_toplevel_decoration_v1_unset_mode(Resource *resource)
{
    Q_UNUSED(resource);
    m_clientPreferredMode = 0;
    handleClientPreferredModeChanged();
}

void WaylandXdgToplevelDecorationV1::handleClientPreferredModeChanged()
{
    if (m_clientPreferredMode != m_configuredMode) {
        if (m_clientPreferredMode == 0)
            sendConfigure(m_manager->preferredMode());
        else
            sendConfigure(DecorationMode(m_clientPreferredMode));
    }
}

} // namespace Compositor

} // namespace Aurora
