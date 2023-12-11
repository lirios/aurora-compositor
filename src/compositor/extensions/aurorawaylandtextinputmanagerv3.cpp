// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "aurorawaylandtextinputmanagerv3.h"
#include "aurorawaylandtextinputmanagerv3_p.h"

#include "aurorawaylandcompositor.h"
#include "aurorawaylandseat.h"
#include "aurorawaylandtextinputv3.h"

namespace Aurora {

namespace Compositor {

WaylandTextInputManagerV3Private::WaylandTextInputManagerV3Private()
{
}

void WaylandTextInputManagerV3Private::zwp_text_input_manager_v3_get_text_input(Resource *resource, uint32_t id, struct ::wl_resource *seatResource)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    Q_Q(WaylandTextInputManagerV3);
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(q->extensionContainer());
    WaylandSeat *seat = WaylandSeat::fromSeatResource(seatResource);
    WaylandTextInputV3 *textInput = WaylandTextInputV3::findIn(seat);
    if (!textInput) {
        textInput = new WaylandTextInputV3(seat, compositor);
    }
    textInput->add(resource->client(), id, wl_resource_get_version(resource->handle));
    WaylandClient *client = WaylandClient::fromWlClient(compositor, resource->client());
    WaylandClient::TextInputProtocols p = client->textInputProtocols();
    client->setTextInputProtocols(p.setFlag(WaylandClient::TextInputProtocol::TextInputV3));
    if (!textInput->isInitialized())
        textInput->initialize();
}

/*!
  \internal
  \preliminary

  \qmltype TextInputManagerV3
  \instantiates WaylandTextInputManagerV3
  \inqmlmodule Aurora.Compositor
  \brief Provides access to input methods in the compositor.

  The \c TextInputManagerV3 corresponds to the \c zwp_text_input_manager_v3 interface
  in the \c text_input_unstable_v3 extension protocol.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.

  \note This protocol is currently a work-in-progress and only exists in Qt for validation purposes. It may change at any time.
*/

/*!
  \internal
  \preliminary
  \class WaylandTextInputManagerV3
  \inmodule AuroraCompositor
  \brief Provides access to input methods in the compositor.

  The \c WaylandTextInputManagerV3 corresponds to the \c zwp_text_input_manager_v3 interface
  in the \c text_input_unstable_v3 extension protocol.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.
  \note This protocol is currently a work-in-progress and only exists in Qt for validation purposes. It may change at any time.
*/

WaylandTextInputManagerV3::WaylandTextInputManagerV3()
    : WaylandCompositorExtensionTemplate<WaylandTextInputManagerV3>(*new WaylandTextInputManagerV3Private)
{
}

WaylandTextInputManagerV3::WaylandTextInputManagerV3(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate<WaylandTextInputManagerV3>(compositor, *new WaylandTextInputManagerV3Private)
{
}

WaylandTextInputManagerV3::~WaylandTextInputManagerV3()
{
}

void WaylandTextInputManagerV3::initialize()
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    Q_D(WaylandTextInputManagerV3);

    WaylandCompositorExtensionTemplate::initialize();
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find WaylandCompositor when initializing WaylandTextInputManagerV3";
        return;
    }
    d->init(compositor->display(), 1);
}

const wl_interface *WaylandTextInputManagerV3::interface()
{
    return WaylandTextInputManagerV3Private::interface();
}

QByteArray WaylandTextInputManagerV3::interfaceName()
{
    return WaylandTextInputManagerV3Private::interfaceName();
}

} // namespace Compositor

} // namespace Aurora
