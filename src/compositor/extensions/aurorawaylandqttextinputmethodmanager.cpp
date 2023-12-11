// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandqttextinputmethodmanager.h"
#include "aurorawaylandqttextinputmethodmanager_p.h"

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandSeat>

#include "aurorawaylandqttextinputmethod.h"

namespace Aurora {

namespace Compositor {

WaylandQtTextInputMethodManagerPrivate::WaylandQtTextInputMethodManagerPrivate()
{
}

void WaylandQtTextInputMethodManagerPrivate::text_input_method_manager_v1_get_text_input_method(Resource *resource, uint32_t id, struct ::wl_resource *seatResource)
{
    Q_Q(WaylandQtTextInputMethodManager);
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(q->extensionContainer());
    WaylandSeat *seat = WaylandSeat::fromSeatResource(seatResource);
    WaylandQtTextInputMethod *textInput = WaylandQtTextInputMethod::findIn(seat);
    if (textInput == nullptr)
        textInput = new WaylandQtTextInputMethod(seat, compositor);
    textInput->add(resource->client(), id, wl_resource_get_version(resource->handle));
    WaylandClient *client = WaylandClient::fromWlClient(compositor, resource->client());
    WaylandClient::TextInputProtocols p = client->textInputProtocols();
    client->setTextInputProtocols(p.setFlag(WaylandClient::TextInputProtocol::QtTextInputMethodV1));

    if (!textInput->isInitialized())
        textInput->initialize();
}

/*!
  \qmltype QtTextInputMethodManager
  \instantiates WaylandQtTextInputMethodManager
  \inqmlmodule Aurora.Compositor
  \since 6.0
  \brief Provides access to input methods in the compositor.

  The \c QtTextInputMethodManager corresponds to the \c qt-text-input-method-manager interface
  in the \c qt-text-input-method-unstable-v1 extension protocol. It is specifically designed
  to be used with a Qt-based input method, such as Qt Virtual Keyboard.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.
*/

/*!
   \class WaylandQtTextInputMethodManager
   \inmodule AuroraCompositor
   \since 6.0
   \brief Provides access to input methods in the compositor.

   The \c WaylandQtTextInputMethodManager class corresponds to the \c qt-text-input-method-manager interface
   in the \c qt-text-input-method-unstable-v1 extension protocol. It is specifically designed
   to be used with a Qt-based input method, such as Qt Virtual Keyboard.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.
*/

WaylandQtTextInputMethodManager::WaylandQtTextInputMethodManager()
    : WaylandCompositorExtensionTemplate<WaylandQtTextInputMethodManager>(*new WaylandQtTextInputMethodManagerPrivate)
{
}

WaylandQtTextInputMethodManager::WaylandQtTextInputMethodManager(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate<WaylandQtTextInputMethodManager>(compositor, *new WaylandQtTextInputMethodManagerPrivate)
{
}

void WaylandQtTextInputMethodManager::initialize()
{
    Q_D(WaylandQtTextInputMethodManager);

    WaylandCompositorExtensionTemplate::initialize();
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (compositor == nullptr) {
        qWarning() << "Failed to find WaylandCompositor when initializing WaylandQtTextInputMethodManager";
        return;
    }

    d->init(compositor->display(), 1);
}

const wl_interface *WaylandQtTextInputMethodManager::interface()
{
    return WaylandQtTextInputMethodManagerPrivate::interface();
}

QByteArray WaylandQtTextInputMethodManager::interfaceName()
{
    return WaylandQtTextInputMethodManagerPrivate::interfaceName();
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandqttextinputmethodmanager.cpp"
