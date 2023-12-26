/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:QTLGPL$
 *
 * GNU Lesser General Public License Usage
 * This file may be used under the terms of the GNU Lesser General
 * Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or (at your option) the GNU General
 * Public license version 3 or any later version approved by the KDE Free
 * Qt Foundation. The licenses are as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPLv2 and LICENSE.GPLv3
 * included in the packaging of this file. Please review the following
 * information to ensure the GNU General Public License requirements will
 * be met: https://www.gnu.org/licenses/gpl-2.0.html and
 * https://www.gnu.org/licenses/gpl-3.0.html.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCore/QTextCodec>
#include <QtCore/QTimer>
#include <QtGui/qpa/qwindowsysteminterface.h>

#include <LiriAuroraLogind/Logind>

#include <xkbcommon/xkbcommon.h>
#include <LiriAuroraXkbCommonSupport/private/auroraxkbcommon_p.h>

#include "libinputhandler_p.h"
#include "libinputkeyboard.h"
#include "libinputkeyboard_p.h"

namespace Aurora {

namespace PlatformSupport {

/*
 * LibInputKeyboardPrivate
 */

LibInputKeyboardPrivate::LibInputKeyboardPrivate(LibInputKeyboard *self, LibInputHandler *h)
    : q_ptr(self)
    , handler(h)
    , context(nullptr)
    , keymap(nullptr)
    , state(nullptr)
    , repeatRate(40)
    , repeatDelay(400)
{
    context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!context) {
        qCWarning(gLcLibinput) << "Unable to create xkb context";
        return;
    }

    keymap = xkb_keymap_new_from_names(context, nullptr,
                                       XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!keymap) {
        qCWarning(gLcLibinput) << "Unable to compile xkb keymap";
        xkb_context_unref(context);
        context = nullptr;
        return;
    }

    state = xkb_state_new(keymap);
    if (!state) {
        qCWarning(gLcLibinput) << "Unable to create xkb state";
        xkb_keymap_unref(keymap);
        keymap = nullptr;
        xkb_context_unref(context);
        context = nullptr;
        return;
    }

    modifiers[0] = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_CTRL);
    modifiers[1] = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_ALT);
    modifiers[2] = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_SHIFT);
    modifiers[3] = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_LOGO);

    repeatTimer.setSingleShot(true);
}

LibInputKeyboardPrivate::~LibInputKeyboardPrivate()
{
    if (state)
        xkb_state_unref(state);
    if (keymap)
        xkb_keymap_unref(keymap);
    if (context)
        xkb_context_unref(context);
}

/*
 * LibInputKeyboard
 */

LibInputKeyboard::LibInputKeyboard(LibInputHandler *handler, QObject *parent)
    : QObject(parent)
    , d_ptr(new LibInputKeyboardPrivate(this, handler))
{
    Q_D(LibInputKeyboard);
    connect(&d->repeatTimer, &QTimer::timeout, this, &LibInputKeyboard::handleRepeat);
}

LibInputKeyboard::~LibInputKeyboard()
{
}

void LibInputKeyboard::handleKey(libinput_event_keyboard *event)
{
    Q_D(LibInputKeyboard);

    if (!d->context || !d->keymap || !d->state)
        return;

    const quint32 key = libinput_event_keyboard_get_key(event) + 8;
    const xkb_keysym_t keysym = xkb_state_key_get_one_sym(d->state, key);
    const bool isPressed = libinput_event_keyboard_get_key_state(event) == LIBINPUT_KEY_STATE_PRESSED;

    Qt::KeyboardModifiers modifiers = XkbCommon::modifiers(d->state);

    const QString text = XkbCommon::lookupString(d->state, key);
    const int qtkey = XkbCommon::keysymToQtKey(keysym, modifiers, d->state, key);

    xkb_state_update_key(d->state, key, isPressed ? XKB_KEY_DOWN : XKB_KEY_UP);

    // Switch to virtual terminal
    if (isPressed && modifiers.testFlag(Qt::ControlModifier) && modifiers.testFlag(Qt::AltModifier)) {
        if (keysym >= XKB_KEY_XF86Switch_VT_1 && keysym <= XKB_KEY_XF86Switch_VT_12)
            Logind::instance()->switchTo(keysym - XKB_KEY_XF86Switch_VT_1 + 1);
    }

    // Event
    LibInputKeyEvent keyEvent;
    keyEvent.key = qtkey;
    keyEvent.modifiers = modifiers;
    keyEvent.nativeScanCode = key;
    keyEvent.nativeVirtualKey = keysym;
    keyEvent.nativeModifiers = modifiers;
    keyEvent.text = text;
    keyEvent.autoRepeat = false;
    keyEvent.repeatCount = 1;
    if (isPressed)
        Q_EMIT d->handler->keyPressed(keyEvent);
    else
        Q_EMIT d->handler->keyReleased(keyEvent);

    // Repeat
    if (isPressed && xkb_keymap_key_repeats(d->keymap, key)) {
        d->repeatData.key = qtkey;
        d->repeatData.modifiers = modifiers;
        d->repeatData.nativeScanCode = key;
        d->repeatData.nativeVirtualKey = keysym;
        d->repeatData.nativeModifiers = modifiers;
        d->repeatData.text = text;
        d->repeatData.repeatCount = 1;
        d->repeatTimer.setInterval(d->repeatDelay);
        d->repeatTimer.start();
    } else if (d->repeatTimer.isActive()) {
        d->repeatTimer.stop();
    }
}

void LibInputKeyboard::handleRepeat()
{
    Q_D(LibInputKeyboard);

    LibInputKeyEvent keyEvent;
    keyEvent.key = d->repeatData.key;
    keyEvent.modifiers = d->repeatData.modifiers;
    keyEvent.nativeScanCode = d->repeatData.nativeScanCode;
    keyEvent.nativeVirtualKey = d->repeatData.nativeVirtualKey;
    keyEvent.nativeModifiers = d->repeatData.nativeModifiers;
    keyEvent.text = d->repeatData.text;
    keyEvent.autoRepeat = true;
    keyEvent.repeatCount = d->repeatData.repeatCount;
    Q_EMIT d->handler->keyPressed(keyEvent);

    ++d->repeatData.repeatCount;
    d->repeatTimer.setInterval(d->repeatRate);
    d->repeatTimer.start();
}

} // namespace PlatformSupport

} // namespace Aurora
