// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandinputmethodcontrol.h"
#include "aurorawaylandinputmethodcontrol_p.h"

#include "aurorawaylandcompositor.h"
#include "aurorawaylandseat.h"
#include "aurorawaylandsurface.h"
#include "aurorawaylandview.h"
#include "aurorawaylandtextinput.h"
#include "aurorawaylandtextinputv3.h"
#include "aurorawaylandqttextinputmethod.h"

#include <QtGui/QInputMethodEvent>

namespace Aurora {

namespace Compositor {

WaylandInputMethodControl::WaylandInputMethodControl(WaylandSurface *surface)
    : QObject(*new WaylandInputMethodControlPrivate(surface), surface)
{
    connect(d_func()->compositor, &WaylandCompositor::defaultSeatChanged,
            this, &WaylandInputMethodControl::defaultSeatChanged);

    updateTextInput();

    WaylandTextInputV3 *textInputV3 = d_func()->textInputV3();
    if (textInputV3) {
        connect(textInputV3, &WaylandTextInputV3::surfaceEnabled, this, &WaylandInputMethodControl::surfaceEnabled);
        connect(textInputV3, &WaylandTextInputV3::surfaceDisabled, this, &WaylandInputMethodControl::surfaceDisabled);
        connect(textInputV3, &WaylandTextInputV3::updateInputMethod, this, &WaylandInputMethodControl::updateInputMethod);
    }

    WaylandQtTextInputMethod *textInputMethod = d_func()->textInputMethod();
    if (textInputMethod) {
        connect(textInputMethod, &WaylandQtTextInputMethod::surfaceEnabled, this, &WaylandInputMethodControl::surfaceEnabled);
        connect(textInputMethod, &WaylandQtTextInputMethod::surfaceDisabled, this, &WaylandInputMethodControl::surfaceDisabled);
        connect(textInputMethod, &WaylandQtTextInputMethod::updateInputMethod, this, &WaylandInputMethodControl::updateInputMethod);
    }
}

QVariant WaylandInputMethodControl::inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const
{
    Q_D(const WaylandInputMethodControl);

    WaylandTextInput *textInput = d->textInput();
    if (textInput != nullptr && textInput->focus() == d->surface)
        return textInput->inputMethodQuery(query, argument);

    WaylandTextInputV3 *textInputV3 = d->textInputV3();
    if (textInputV3 != nullptr && textInputV3->focus() == d->surface)
        return textInputV3->inputMethodQuery(query, argument);

    WaylandQtTextInputMethod *textInputMethod = d_func()->textInputMethod();
    if (textInputMethod && textInputMethod->focusedSurface() == d->surface)
        return textInputMethod->inputMethodQuery(query, argument);

    return QVariant();
}

void WaylandInputMethodControl::inputMethodEvent(QInputMethodEvent *event)
{
    Q_D(WaylandInputMethodControl);

    if (WaylandTextInput *textInput = d->textInput()) {
        textInput->sendInputMethodEvent(event);
    } else if (WaylandTextInputV3 *textInputV3 = d->textInputV3()) {
        textInputV3->sendInputMethodEvent(event);
    } else if (WaylandQtTextInputMethod *textInputMethod = d->textInputMethod()) {
        textInputMethod->sendInputMethodEvent(event);
    } else {
        event->ignore();
    }
}

bool WaylandInputMethodControl::enabled() const
{
    Q_D(const WaylandInputMethodControl);

    return d->enabled;
}

void WaylandInputMethodControl::setEnabled(bool enabled)
{
    Q_D(WaylandInputMethodControl);

    if (d->enabled == enabled)
        return;

    d->enabled = enabled;
    emit enabledChanged(enabled);
    emit updateInputMethod(Qt::ImQueryInput);
}

void WaylandInputMethodControl::surfaceEnabled(WaylandSurface *surface)
{
    Q_D(WaylandInputMethodControl);

    if (surface == d->surface)
        setEnabled(true);
}

void WaylandInputMethodControl::surfaceDisabled(WaylandSurface *surface)
{
    Q_D(WaylandInputMethodControl);

    if (surface == d->surface)
        setEnabled(false);
}

void WaylandInputMethodControl::setSurface(WaylandSurface *surface)
{
    Q_D(WaylandInputMethodControl);

    if (d->surface == surface)
        return;

    d->surface = surface;

    WaylandTextInput *textInput = d->textInput();
    WaylandTextInputV3 *textInputV3 = d->textInputV3();
    WaylandQtTextInputMethod *textInputMethod = d->textInputMethod();
    setEnabled((textInput && textInput->isSurfaceEnabled(d->surface))
               || (textInputV3 && textInputV3->isSurfaceEnabled(d->surface))
               || (textInputMethod && textInputMethod->isSurfaceEnabled(d->surface)));
}

void WaylandInputMethodControl::updateTextInput()
{
    WaylandTextInput *textInput = d_func()->textInput();

    if (textInput) {
        connect(textInput, &WaylandTextInput::surfaceEnabled, this, &WaylandInputMethodControl::surfaceEnabled, Qt::UniqueConnection);
        connect(textInput, &WaylandTextInput::surfaceDisabled, this, &WaylandInputMethodControl::surfaceDisabled, Qt::UniqueConnection);
        connect(textInput, &WaylandTextInput::updateInputMethod, this, &WaylandInputMethodControl::updateInputMethod, Qt::UniqueConnection);
    }
}

void WaylandInputMethodControl::defaultSeatChanged()
{
    Q_D(WaylandInputMethodControl);

    disconnect(d->textInput(), nullptr, this, nullptr);
    disconnect(d->textInputV3(), nullptr, this, nullptr);
    disconnect(d->textInputMethod(), nullptr, this, nullptr);

    d->seat = d->compositor->defaultSeat();
    WaylandTextInput *textInput = d->textInput();
    WaylandTextInputV3 *textInputV3 = d->textInputV3();
    WaylandQtTextInputMethod *textInputMethod = d->textInputMethod();

    if (textInput) {
        connect(textInput, &WaylandTextInput::surfaceEnabled, this, &WaylandInputMethodControl::surfaceEnabled);
        connect(textInput, &WaylandTextInput::surfaceDisabled, this, &WaylandInputMethodControl::surfaceDisabled);
    }

    if (textInputV3) {
        connect(textInputV3, &WaylandTextInputV3::surfaceEnabled, this, &WaylandInputMethodControl::surfaceEnabled);
        connect(textInputV3, &WaylandTextInputV3::surfaceDisabled, this, &WaylandInputMethodControl::surfaceDisabled);
    }

    if (textInputMethod) {
        connect(textInputMethod, &WaylandQtTextInputMethod::surfaceEnabled, this, &WaylandInputMethodControl::surfaceEnabled);
        connect(textInputMethod, &WaylandQtTextInputMethod::surfaceDisabled, this, &WaylandInputMethodControl::surfaceDisabled);
    }

    setEnabled((textInput && textInput->isSurfaceEnabled(d->surface))
               || (textInputV3 && textInputV3->isSurfaceEnabled(d->surface))
               || (textInputMethod && textInputMethod->isSurfaceEnabled(d->surface)));
}

WaylandInputMethodControlPrivate::WaylandInputMethodControlPrivate(WaylandSurface *surface)
    : compositor(surface->compositor())
    , seat(compositor->defaultSeat())
    , surface(surface)
{
}

WaylandQtTextInputMethod *WaylandInputMethodControlPrivate::textInputMethod() const
{
    if (!surface->client() || !surface->client()->textInputProtocols().testFlag(WaylandClient::TextInputProtocol::QtTextInputMethodV1))
        return nullptr;
    return WaylandQtTextInputMethod::findIn(seat);
}

WaylandTextInput *WaylandInputMethodControlPrivate::textInput() const
{
    if (!surface->client() || !surface->client()->textInputProtocols().testFlag(WaylandClient::TextInputProtocol::TextInputV2))
        return nullptr;
    return WaylandTextInput::findIn(seat);
}

WaylandTextInputV3 *WaylandInputMethodControlPrivate::textInputV3() const
{
    return WaylandTextInputV3::findIn(seat);
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandinputmethodcontrol.cpp"
