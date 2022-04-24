/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "aurorawaylandtextinputmanagerv4.h"
#include "aurorawaylandtextinputmanagerv4_p.h"

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandSeat>

#include "aurorawaylandtextinputv4.h"

namespace Aurora {

namespace Compositor {

WaylandTextInputManagerV4Private::WaylandTextInputManagerV4Private()
{
}

void WaylandTextInputManagerV4Private::zwp_text_input_manager_v4_get_text_input(Resource *resource, uint32_t id, struct ::wl_resource *seatResource)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    Q_Q(WaylandTextInputManagerV4);
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(q->extensionContainer());
    WaylandSeat *seat = WaylandSeat::fromSeatResource(seatResource);
    WaylandTextInputV4 *textInput = WaylandTextInputV4::findIn(seat);
    if (!textInput) {
        textInput = new WaylandTextInputV4(seat, compositor);
    }
    textInput->add(resource->client(), id, wl_resource_get_version(resource->handle));
    WaylandClient *client = WaylandClient::fromWlClient(compositor, resource->client());
    WaylandClient::TextInputProtocols p = client->textInputProtocols();
    client->setTextInputProtocols(p.setFlag(WaylandClient::TextInputProtocol::TextInputV4));
    if (!textInput->isInitialized())
        textInput->initialize();
}

/*!
  \internal
  \preliminary

  \qmltype TextInputManagerV4
  \instantiates WaylandTextInputManagerV4
  \inqmlmodule Aurora.Compositor
  \brief Provides access to input methods in the compositor.

  The \c TextInputManagerV4 corresponds to the \c zwp_text_input_manager_v4 interface
  in the \c text_input_unstable_v4 extension protocol.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.

  \note This protocol is currently a work-in-progress and only exists in Qt for validation purposes. It may change at any time.
*/

/*!
  \internal
  \preliminary
  \class WaylandTextInputManagerV4
  \inmodule AuroraCompositor
  \brief Provides access to input methods in the compositor.

  The \c WaylandTextInputManagerV4 corresponds to the \c zwp_text_input_manager_v4 interface
  in the \c text_input_unstable_v4 extension protocol.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.
  \note This protocol is currently a work-in-progress and only exists in Qt for validation purposes. It may change at any time.
*/

WaylandTextInputManagerV4::WaylandTextInputManagerV4()
    : WaylandCompositorExtensionTemplate<WaylandTextInputManagerV4>(*new WaylandTextInputManagerV4Private)
{
}

WaylandTextInputManagerV4::WaylandTextInputManagerV4(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate<WaylandTextInputManagerV4>(compositor, *new WaylandTextInputManagerV4Private)
{
}

WaylandTextInputManagerV4::~WaylandTextInputManagerV4()
{
}

void WaylandTextInputManagerV4::initialize()
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    Q_D(WaylandTextInputManagerV4);

    WaylandCompositorExtensionTemplate::initialize();
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find WaylandCompositor when initializing WaylandTextInputManagerV4";
        return;
    }
    d->init(compositor->display(), 1);
}

const wl_interface *WaylandTextInputManagerV4::interface()
{
    return WaylandTextInputManagerV4Private::interface();
}

QByteArray WaylandTextInputManagerV4::interfaceName()
{
    return WaylandTextInputManagerV4Private::interfaceName();
}

} // namespace Compositor

} // namespace Aurora
