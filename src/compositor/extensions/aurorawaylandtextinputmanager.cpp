// Copyright (C) 2017-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandtextinputmanager.h"
#include "aurorawaylandtextinputmanager_p.h"

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandSeat>

#include "aurorawaylandtextinput.h"

namespace Aurora {

namespace Compositor {

WaylandTextInputManagerPrivate::WaylandTextInputManagerPrivate(WaylandTextInputManager *self)
    : WaylandCompositorExtensionPrivate(self)
{
}

void WaylandTextInputManagerPrivate::zwp_text_input_manager_v2_get_text_input(Resource *resource, uint32_t id, struct ::wl_resource *seatResource)
{
    Q_Q(WaylandTextInputManager);
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(q->extensionContainer());
    WaylandSeat *seat = WaylandSeat::fromSeatResource(seatResource);
    WaylandTextInput *textInput = WaylandTextInput::findIn(seat);
    if (!textInput)
        textInput = new WaylandTextInput(seat, compositor);
    textInput->add(resource->client(), id, wl_resource_get_version(resource->handle));
    WaylandClient *client = WaylandClient::fromWlClient(compositor, resource->client());
    WaylandClient::TextInputProtocols p = client->textInputProtocols();
    client->setTextInputProtocols(p.setFlag(WaylandClient::TextInputProtocol::TextInputV2));
    if (!textInput->isInitialized())
        textInput->initialize();
}

/*!
  \qmltype TextInputManager
  \instantiates WaylandTextInputManager
  \inqmlmodule Aurora.Compositor
  \brief Provides access to input methods in the compositor.

  The \c TextInputManager corresponds to the \c zwp_text_input_manager_v2 interface
  in the \c text_input_unstable_v2 extension protocol.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.
*/

/*!
  \class WaylandTextInputManager
  \inmodule AuroraCompositor
  \brief Provides access to input methods in the compositor.

  The \c WaylandTextInputManager corresponds to the \c zwp_text_input_manager_v2 interface
  in the \c text_input_unstable_v2 extension protocol.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.
*/

WaylandTextInputManager::WaylandTextInputManager()
    : WaylandCompositorExtensionTemplate<WaylandTextInputManager>()
    , d_ptr(new WaylandTextInputManagerPrivate(this))
{
}

WaylandTextInputManager::WaylandTextInputManager(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate<WaylandTextInputManager>(compositor)
    , d_ptr(new WaylandTextInputManagerPrivate(this))
{
}

WaylandTextInputManager::~WaylandTextInputManager()
{
}

void WaylandTextInputManager::initialize()
{
    Q_D(WaylandTextInputManager);

    WaylandCompositorExtensionTemplate::initialize();
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find WaylandCompositor when initializing WaylandTextInputManager";
        return;
    }
    d->init(compositor->display(), 1);
}

const wl_interface *WaylandTextInputManager::interface()
{
    return WaylandTextInputManagerPrivate::interface();
}

QByteArray WaylandTextInputManager::interfaceName()
{
    return WaylandTextInputManagerPrivate::interfaceName();
}

} // namespace Compositor

} // namespace Aurora
