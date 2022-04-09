/****************************************************************************
**
** Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#include "aurorawaylandinputmethodcontrol.h"
#include "aurorawaylandinputmethodcontrol_p.h"

#include "aurorawaylandcompositor.h"
#include "aurorawaylandseat.h"
#include "aurorawaylandsurface.h"
#include "aurorawaylandview.h"
#include "aurorawaylandtextinput.h"
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
#include "aurorawaylandtextinputv4.h"
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
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

#if QT_WAYLAND_TEXT_INPUT_V4_WIP
    WaylandTextInputV4 *textInputV4 = d_func()->textInputV4();
    if (textInputV4) {
        connect(textInputV4, &WaylandTextInputV4::surfaceEnabled, this, &WaylandInputMethodControl::surfaceEnabled);
        connect(textInputV4, &WaylandTextInputV4::surfaceDisabled, this, &WaylandInputMethodControl::surfaceDisabled);
        connect(textInputV4, &WaylandTextInputV4::updateInputMethod, this, &WaylandInputMethodControl::updateInputMethod);
    }
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP

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

#if QT_WAYLAND_TEXT_INPUT_V4_WIP
    WaylandTextInputV4 *textInputV4 = d->textInputV4();
    if (textInputV4 != nullptr && textInputV4->focus() == d->surface)
        return textInputV4->inputMethodQuery(query, argument);
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP

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
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
    } else if (WaylandTextInputV4 *textInputV4 = d->textInputV4()) {
        textInputV4->sendInputMethodEvent(event);
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
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
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
    WaylandTextInputV4 *textInputV4 = d->textInputV4();
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
    WaylandQtTextInputMethod *textInputMethod = d->textInputMethod();
    setEnabled((textInput && textInput->isSurfaceEnabled(d->surface))
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
               || (textInputV4 && textInputV4->isSurfaceEnabled(d->surface))
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
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
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
    disconnect(d->textInputV4(), nullptr, this, nullptr);
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
    disconnect(d->textInputMethod(), nullptr, this, nullptr);

    d->seat = d->compositor->defaultSeat();
    WaylandTextInput *textInput = d->textInput();
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
    WaylandTextInputV4 *textInputV4 = d->textInputV4();
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
    WaylandQtTextInputMethod *textInputMethod = d->textInputMethod();

    if (textInput) {
        connect(textInput, &WaylandTextInput::surfaceEnabled, this, &WaylandInputMethodControl::surfaceEnabled);
        connect(textInput, &WaylandTextInput::surfaceDisabled, this, &WaylandInputMethodControl::surfaceDisabled);
    }

#if QT_WAYLAND_TEXT_INPUT_V4_WIP
    if (textInputV4) {
        connect(textInputV4, &WaylandTextInputV4::surfaceEnabled, this, &WaylandInputMethodControl::surfaceEnabled);
        connect(textInputV4, &WaylandTextInputV4::surfaceDisabled, this, &WaylandInputMethodControl::surfaceDisabled);
    }
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP

    if (textInputMethod) {
        connect(textInputMethod, &WaylandQtTextInputMethod::surfaceEnabled, this, &WaylandInputMethodControl::surfaceEnabled);
        connect(textInputMethod, &WaylandQtTextInputMethod::surfaceDisabled, this, &WaylandInputMethodControl::surfaceDisabled);
    }

    setEnabled((textInput && textInput->isSurfaceEnabled(d->surface))
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
               || (textInputV4 && textInputV4->isSurfaceEnabled(d->surface))
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
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
    if (!surface->client()->textInputProtocols().testFlag(WaylandClient::TextInputProtocol::QtTextInputMethodV1))
        return nullptr;
    return WaylandQtTextInputMethod::findIn(seat);
}

WaylandTextInput *WaylandInputMethodControlPrivate::textInput() const
{
    if (!surface->client()->textInputProtocols().testFlag(WaylandClient::TextInputProtocol::TextInputV2))
        return nullptr;
    return WaylandTextInput::findIn(seat);
}

#if QT_WAYLAND_TEXT_INPUT_V4_WIP
WaylandTextInputV4 *WaylandInputMethodControlPrivate::textInputV4() const
{
    return WaylandTextInputV4::findIn(seat);
}
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandinputmethodcontrol.cpp"
