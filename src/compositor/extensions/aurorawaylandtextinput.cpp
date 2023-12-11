// Copyright (C) 2017-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandtextinput.h"
#include "aurorawaylandtextinput_p.h"

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/private/aurorawaylandseat_p.h>

#include "aurorawaylandsurface.h"
#include "aurorawaylandview.h"
#include "aurorawaylandinputmethodeventbuilder_p.h"
#include "aurorawaylandinputmethodcontrol.h"

#include <QGuiApplication>
#include <QInputMethodEvent>

#if LIRI_FEATURE_aurora_xkbcommon
#include <LiriAuroraXkbCommonSupport/private/auroraxkbcommon_p.h>
#endif

namespace Aurora {

namespace Compositor {

WaylandTextInputClientState::WaylandTextInputClientState()
{
}

Qt::InputMethodQueries WaylandTextInputClientState::updatedQueries(const WaylandTextInputClientState &other) const
{
    Qt::InputMethodQueries queries;

    if (hints != other.hints)
        queries |= Qt::ImHints;
    if (cursorRectangle != other.cursorRectangle)
        queries |= Qt::ImCursorRectangle;
    if (surroundingText != other.surroundingText)
        queries |= Qt::ImSurroundingText | Qt::ImCurrentSelection;
    if (cursorPosition != other.cursorPosition)
        queries |= Qt::ImCursorPosition | Qt::ImCurrentSelection;
    if (anchorPosition != other.anchorPosition)
        queries |= Qt::ImAnchorPosition | Qt::ImCurrentSelection;
    if (preferredLanguage != other.preferredLanguage)
        queries |= Qt::ImPreferredLanguage;

    return queries;
}

Qt::InputMethodQueries WaylandTextInputClientState::mergeChanged(const WaylandTextInputClientState &other) {
    Qt::InputMethodQueries queries;

    if ((other.changedState & Qt::ImHints) && hints != other.hints) {
        hints = other.hints;
        queries |= Qt::ImHints;
    }

    if ((other.changedState & Qt::ImCursorRectangle) && cursorRectangle != other.cursorRectangle) {
        cursorRectangle = other.cursorRectangle;
        queries |= Qt::ImCursorRectangle;
    }

    if ((other.changedState & Qt::ImSurroundingText) && surroundingText != other.surroundingText) {
        surroundingText = other.surroundingText;
        queries |= Qt::ImSurroundingText | Qt::ImCurrentSelection;
    }

    if ((other.changedState & Qt::ImCursorPosition) && cursorPosition != other.cursorPosition) {
        cursorPosition = other.cursorPosition;
        queries |= Qt::ImCursorPosition | Qt::ImCurrentSelection;
    }

    if ((other.changedState & Qt::ImAnchorPosition) && anchorPosition != other.anchorPosition) {
        anchorPosition = other.anchorPosition;
        queries |= Qt::ImAnchorPosition | Qt::ImCurrentSelection;
    }

    if ((other.changedState & Qt::ImPreferredLanguage) && preferredLanguage != other.preferredLanguage) {
        preferredLanguage = other.preferredLanguage;
        queries |= Qt::ImPreferredLanguage;
    }

    return queries;
}

WaylandTextInputPrivate::WaylandTextInputPrivate(WaylandCompositor *compositor)
    : compositor(compositor)
    , currentState(new WaylandTextInputClientState)
    , pendingState(new WaylandTextInputClientState)
{
}

void WaylandTextInputPrivate::sendInputMethodEvent(QInputMethodEvent *event)
{
    Q_Q(WaylandTextInput);

    if (!focusResource || !focusResource->handle)
        return;

    WaylandTextInputClientState afterCommit;

    afterCommit.surroundingText = currentState->surroundingText;
    afterCommit.cursorPosition = qMin(currentState->cursorPosition, currentState->anchorPosition);

    // Remove selection
    afterCommit.surroundingText.remove(afterCommit.cursorPosition, qAbs(currentState->cursorPosition - currentState->anchorPosition));

    if (event->replacementLength() > 0 || event->replacementStart() != 0) {
        // Remove replacement
        afterCommit.cursorPosition = qBound(0, afterCommit.cursorPosition + event->replacementStart(), afterCommit.surroundingText.size());
        afterCommit.surroundingText.remove(afterCommit.cursorPosition,
                                           qMin(event->replacementLength(),
                                                afterCommit.surroundingText.size() - afterCommit.cursorPosition));

        if (event->replacementStart() <= 0 && (event->replacementLength() >= -event->replacementStart())) {
            const int selectionStart = qMin(currentState->cursorPosition, currentState->anchorPosition);
            const int selectionEnd = qMax(currentState->cursorPosition, currentState->anchorPosition);
            const int before = WaylandInputMethodEventBuilder::indexToWayland(currentState->surroundingText, -event->replacementStart(), selectionStart + event->replacementStart());
            const int after = WaylandInputMethodEventBuilder::indexToWayland(currentState->surroundingText, event->replacementLength() + event->replacementStart(), selectionEnd);
            send_delete_surrounding_text(focusResource->handle, before, after);
        } else {
            // TODO: Implement this case
            qWarning() << "Not yet supported case of replacement. Start:" << event->replacementStart() << "length:" << event->replacementLength();
        }
    }

    // Insert commit string
    afterCommit.surroundingText.insert(afterCommit.cursorPosition, event->commitString());
    afterCommit.cursorPosition += event->commitString().size();
    afterCommit.anchorPosition = afterCommit.cursorPosition;

    for (const QInputMethodEvent::Attribute &attribute : event->attributes()) {
        if (attribute.type == QInputMethodEvent::Selection) {
            afterCommit.cursorPosition = attribute.start;
            afterCommit.anchorPosition = attribute.length;
            int cursor = WaylandInputMethodEventBuilder::indexToWayland(afterCommit.surroundingText, qAbs(attribute.start - afterCommit.cursorPosition), qMin(attribute.start, afterCommit.cursorPosition));
            int anchor = WaylandInputMethodEventBuilder::indexToWayland(afterCommit.surroundingText, qAbs(attribute.length - afterCommit.cursorPosition), qMin(attribute.length, afterCommit.cursorPosition));
            send_cursor_position(focusResource->handle,
                                 attribute.start < afterCommit.cursorPosition ? -cursor : cursor,
                                 attribute.length < afterCommit.cursorPosition ? -anchor : anchor);
        }
    }
    send_commit_string(focusResource->handle, event->commitString());
    for (const QInputMethodEvent::Attribute &attribute : event->attributes()) {
        if (attribute.type == QInputMethodEvent::Cursor) {
            int index = WaylandInputMethodEventBuilder::indexToWayland(event->preeditString(), attribute.start);
            send_preedit_cursor(focusResource->handle, index);
        } else if (attribute.type == QInputMethodEvent::TextFormat) {
            int start = WaylandInputMethodEventBuilder::indexToWayland(event->preeditString(), attribute.start);
            int length = WaylandInputMethodEventBuilder::indexToWayland(event->preeditString(), attribute.length, attribute.start);
            // TODO add support for different stylesQWaylandTextInput
            send_preedit_styling(focusResource->handle, start, length, preedit_style_default);
        }
    }
    send_preedit_string(focusResource->handle, event->preeditString(), event->preeditString());

    Qt::InputMethodQueries queries = currentState->updatedQueries(afterCommit);
    currentState->surroundingText = afterCommit.surroundingText;
    currentState->cursorPosition = afterCommit.cursorPosition;
    currentState->anchorPosition = afterCommit.anchorPosition;

    if (queries) {
        qCDebug(gLcAuroraCompositorInputMethods) << "QInputMethod::update() after QInputMethodEvent" << queries;

        emit q->updateInputMethod(queries);
    }
}

void WaylandTextInputPrivate::sendKeyEvent(QKeyEvent *event)
{
    if (!focusResource || !focusResource->handle)
        return;

    uint mods = 0;
    const auto &qtMods = event->modifiers();
    if (qtMods & Qt::ShiftModifier)
        mods |= shiftModifierMask;
    if (qtMods & Qt::ControlModifier)
        mods |= controlModifierMask;
    if (qtMods & Qt::AltModifier)
        mods |= altModifierMask;
    if (qtMods & Qt::MetaModifier)
        mods |= metaModifierMask;

#if LIRI_FEATURE_aurora_xkbcommon
    for (xkb_keysym_t keysym : XkbCommon::toKeysym(event)) {
        send_keysym(focusResource->handle, event->timestamp(), keysym,
                    event->type() == QEvent::KeyPress ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED,
                    mods);
    }
#else
    Q_UNUSED(event);
#endif
}

void WaylandTextInputPrivate::sendInputPanelState()
{
    if (!focusResource || !focusResource->handle)
        return;

    QInputMethod *inputMethod = qApp->inputMethod();
    const QRectF& keyboardRect = inputMethod->keyboardRectangle();
    const QRectF& sceneInputRect = inputMethod->inputItemTransform().mapRect(inputMethod->inputItemRectangle());
    const QRectF& localRect = sceneInputRect.intersected(keyboardRect).translated(-sceneInputRect.topLeft());

    send_input_panel_state(focusResource->handle,
                           inputMethod->isVisible() ? input_panel_visibility_visible : input_panel_visibility_hidden,
                           localRect.x(), localRect.y(), localRect.width(), localRect.height());
}

void WaylandTextInputPrivate::sendTextDirection()
{
    if (!focusResource || !focusResource->handle)
        return;

    const Qt::LayoutDirection direction = qApp->inputMethod()->inputDirection();
    send_text_direction(focusResource->handle,
                        (direction == Qt::LeftToRight) ? text_direction_ltr :
                                                         (direction == Qt::RightToLeft) ? text_direction_rtl : text_direction_auto);
}

void WaylandTextInputPrivate::sendLocale()
{
    if (!focusResource || !focusResource->handle)
        return;

    const QLocale locale = qApp->inputMethod()->locale();
    send_language(focusResource->handle, locale.bcp47Name());
}

QVariant WaylandTextInputPrivate::inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const
{
    switch (property) {
    case Qt::ImHints:
        return QVariant(static_cast<int>(currentState->hints));
    case Qt::ImCursorRectangle:
        return currentState->cursorRectangle;
    case Qt::ImFont:
        // Not supported
        return QVariant();
    case Qt::ImCursorPosition:
        return currentState->cursorPosition;
    case Qt::ImSurroundingText:
        return currentState->surroundingText;
    case Qt::ImCurrentSelection:
        return currentState->surroundingText.mid(qMin(currentState->cursorPosition, currentState->anchorPosition),
                                                 qAbs(currentState->anchorPosition - currentState->cursorPosition));
    case Qt::ImMaximumTextLength:
        // Not supported
        return QVariant();
    case Qt::ImAnchorPosition:
        return currentState->anchorPosition;
    case Qt::ImAbsolutePosition:
        // We assume the surrounding text is our whole document for now
        return currentState->cursorPosition;
    case Qt::ImTextAfterCursor:
        if (argument.isValid())
            return currentState->surroundingText.mid(currentState->cursorPosition, argument.toInt());
        return currentState->surroundingText.mid(currentState->cursorPosition);
    case Qt::ImTextBeforeCursor:
        if (argument.isValid())
            return currentState->surroundingText.left(currentState->cursorPosition).right(argument.toInt());
        return currentState->surroundingText.left(currentState->cursorPosition);
    case Qt::ImPreferredLanguage:
        return currentState->preferredLanguage;

    default:
        return QVariant();
    }
}

void WaylandTextInputPrivate::setFocus(WaylandSurface *surface)
{
    Q_Q(WaylandTextInput);

    if (focusResource && focus != surface) {
        uint32_t serial = compositor->nextSerial();
        send_leave(focusResource->handle, serial, focus->resource());
        focusDestroyListener.reset();
    }

    Resource *resource = surface ? resourceMap().value(surface->waylandClient()) : 0;

    if (resource && (focus != surface || focusResource != resource)) {
        uint32_t serial = compositor->nextSerial();
        currentState.reset(new WaylandTextInputClientState);
        pendingState.reset(new WaylandTextInputClientState);
        send_enter(resource->handle, serial, surface->resource());
        focusResource = resource;
        sendInputPanelState();
        sendLocale();
        sendTextDirection();
        focusDestroyListener.listenForDestruction(surface->resource());
        if (inputPanelVisible && q->isSurfaceEnabled(surface))
            qApp->inputMethod()->show();
    }

    focusResource = resource;
    focus = surface;
}

void WaylandTextInputPrivate::sendModifiersMap(const QByteArray &modifiersMap)
{
    send_modifiers_map(focusResource->handle, modifiersMap);
}

#if !LIRI_FEATURE_aurora_xkbcommon
#define XKB_MOD_NAME_SHIFT   "Shift"
#define XKB_MOD_NAME_CTRL    "Control"
#define XKB_MOD_NAME_ALT     "Mod1"
#define XKB_MOD_NAME_LOGO    "Mod4"
#endif
void WaylandTextInputPrivate::zwp_text_input_v2_bind_resource(Resource *resource)
{
    QByteArray modifiers = XKB_MOD_NAME_SHIFT + QByteArray(1, '\0');
    modifiers += XKB_MOD_NAME_CTRL + QByteArray(1, '\0');
    modifiers += XKB_MOD_NAME_ALT + QByteArray(1, '\0');
    modifiers += XKB_MOD_NAME_LOGO + QByteArray(1, '\0');
    send_modifiers_map(resource->handle, modifiers);
}

void WaylandTextInputPrivate::zwp_text_input_v2_destroy_resource(Resource *resource)
{
    if (focusResource == resource)
        focusResource = nullptr;
}

void WaylandTextInputPrivate::zwp_text_input_v2_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void WaylandTextInputPrivate::zwp_text_input_v2_enable(Resource *resource, wl_resource *surface)
{
    Q_Q(WaylandTextInput);

    WaylandSurface *s = WaylandSurface::fromResource(surface);
    enabledSurfaces.insert(resource, s);

    WaylandInputMethodControl *control = s->inputMethodControl();
    if (control)
        control->updateTextInput();

    emit q->surfaceEnabled(s);
}

void WaylandTextInputPrivate::zwp_text_input_v2_disable(PrivateServer::zwp_text_input_v2::Resource *resource, wl_resource *)
{
    Q_Q(WaylandTextInput);

    WaylandSurface *s = enabledSurfaces.take(resource);
    emit q->surfaceDisabled(s);
}

void WaylandTextInputPrivate::zwp_text_input_v2_show_input_panel(Resource *)
{
    inputPanelVisible = true;

    qApp->inputMethod()->show();
}

void WaylandTextInputPrivate::zwp_text_input_v2_hide_input_panel(Resource *)
{
    inputPanelVisible = false;

    qApp->inputMethod()->hide();
}

void WaylandTextInputPrivate::zwp_text_input_v2_set_cursor_rectangle(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (resource != focusResource)
        return;

    pendingState->cursorRectangle = QRect(x, y, width, height);

    pendingState->changedState |= Qt::ImCursorRectangle;
}

void WaylandTextInputPrivate::zwp_text_input_v2_update_state(Resource *resource, uint32_t serial, uint32_t flags)
{
    Q_Q(WaylandTextInput);

    qCDebug(gLcAuroraCompositorInputMethods) << "update_state" << serial << flags;

    if (resource != focusResource)
        return;

    if (flags == update_state_reset || flags == update_state_enter) {
        qCDebug(gLcAuroraCompositorInputMethods) << "QInputMethod::reset()";
        qApp->inputMethod()->reset();
    }

    this->serial = serial;

    Qt::InputMethodQueries queries;
    if (flags == update_state_change) {
        queries = currentState->mergeChanged(*pendingState);
    } else {
        queries = pendingState->updatedQueries(*currentState);
        currentState.swap(pendingState);
    }

    pendingState.reset(new WaylandTextInputClientState);

    if (queries) {
        qCDebug(gLcAuroraCompositorInputMethods) << "QInputMethod::update()" << queries;

        emit q->updateInputMethod(queries);
    }
}

void WaylandTextInputPrivate::zwp_text_input_v2_set_content_type(Resource *resource, uint32_t hint, uint32_t purpose)
{
    if (resource != focusResource)
        return;

    pendingState->hints = Qt::ImhNone;

    if ((hint & content_hint_auto_completion) == 0
        && (hint & content_hint_auto_correction) == 0)
        pendingState->hints |= Qt::ImhNoPredictiveText;
    if ((hint & content_hint_auto_capitalization) == 0)
        pendingState->hints |= Qt::ImhNoAutoUppercase;
    if ((hint & content_hint_lowercase) != 0)
        pendingState->hints |= Qt::ImhPreferLowercase;
    if ((hint & content_hint_uppercase) != 0)
        pendingState->hints |= Qt::ImhPreferUppercase;
    if ((hint & content_hint_hidden_text) != 0)
        pendingState->hints |= Qt::ImhHiddenText;
    if ((hint & content_hint_sensitive_data) != 0)
        pendingState->hints |= Qt::ImhSensitiveData;
    if ((hint & content_hint_latin) != 0)
        pendingState->hints |= Qt::ImhLatinOnly;
    if ((hint & content_hint_multiline) != 0)
        pendingState->hints |= Qt::ImhMultiLine;

    switch (purpose) {
    case content_purpose_normal:
        break;
    case content_purpose_alpha:
        pendingState->hints |= Qt::ImhUppercaseOnly | Qt::ImhLowercaseOnly;
        break;
    case content_purpose_digits:
        pendingState->hints |= Qt::ImhDigitsOnly;
        break;
    case content_purpose_number:
        pendingState->hints |= Qt::ImhFormattedNumbersOnly;
        break;
    case content_purpose_phone:
        pendingState->hints |= Qt::ImhDialableCharactersOnly;
        break;
    case content_purpose_url:
        pendingState->hints |= Qt::ImhUrlCharactersOnly;
        break;
    case content_purpose_email:
        pendingState->hints |= Qt::ImhEmailCharactersOnly;
        break;
    case content_purpose_name:
    case content_purpose_password:
        break;
    case content_purpose_date:
        pendingState->hints |= Qt::ImhDate;
        break;
    case content_purpose_time:
        pendingState->hints |= Qt::ImhTime;
        break;
    case content_purpose_datetime:
        pendingState->hints |= Qt::ImhDate | Qt::ImhTime;
        break;
    case content_purpose_terminal:
    default:
        break;
    }

    pendingState->changedState |= Qt::ImHints;
}

void WaylandTextInputPrivate::zwp_text_input_v2_set_preferred_language(Resource *resource, const QString &language)
{
    if (resource != focusResource)
        return;

    pendingState->preferredLanguage = language;

    pendingState->changedState |= Qt::ImPreferredLanguage;
}

void WaylandTextInputPrivate::zwp_text_input_v2_set_surrounding_text(Resource *resource, const QString &text, int32_t cursor, int32_t anchor)
{
    if (resource != focusResource)
        return;

    pendingState->surroundingText = text;
    pendingState->cursorPosition = WaylandInputMethodEventBuilder::indexFromWayland(text, cursor);
    pendingState->anchorPosition = WaylandInputMethodEventBuilder::indexFromWayland(text, anchor);

    pendingState->changedState |= Qt::ImSurroundingText | Qt::ImCursorPosition | Qt::ImAnchorPosition;
}

WaylandTextInput::WaylandTextInput(WaylandObject *container, WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate(container, *new WaylandTextInputPrivate(compositor))
{
    connect(&d_func()->focusDestroyListener, &WaylandDestroyListener::fired,
            this, &WaylandTextInput::focusSurfaceDestroyed);

    connect(qApp->inputMethod(), &QInputMethod::visibleChanged,
            this, &WaylandTextInput::sendInputPanelState);
    connect(qApp->inputMethod(), &QInputMethod::keyboardRectangleChanged,
            this, &WaylandTextInput::sendInputPanelState);
    connect(qApp->inputMethod(), &QInputMethod::inputDirectionChanged,
            this, &WaylandTextInput::sendTextDirection);
    connect(qApp->inputMethod(), &QInputMethod::localeChanged,
            this, &WaylandTextInput::sendLocale);
}

WaylandTextInput::~WaylandTextInput()
{
}

void WaylandTextInput::sendInputMethodEvent(QInputMethodEvent *event)
{
    Q_D(WaylandTextInput);

    d->sendInputMethodEvent(event);
}

void WaylandTextInput::sendKeyEvent(QKeyEvent *event)
{
    Q_D(WaylandTextInput);

    d->sendKeyEvent(event);
}

void WaylandTextInput::sendInputPanelState()
{
    Q_D(WaylandTextInput);

    d->sendInputPanelState();
}

void WaylandTextInput::sendTextDirection()
{
    Q_D(WaylandTextInput);

    d->sendTextDirection();
}

void WaylandTextInput::sendLocale()
{
    Q_D(WaylandTextInput);

    d->sendLocale();
}

QVariant WaylandTextInput::inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const
{
    const Q_D(WaylandTextInput);

    return d->inputMethodQuery(property, argument);
}

WaylandSurface *WaylandTextInput::focus() const
{
    const Q_D(WaylandTextInput);

    return d->focus;
}

void WaylandTextInput::setFocus(WaylandSurface *surface)
{
    Q_D(WaylandTextInput);

    d->setFocus(surface);
}

void WaylandTextInput::focusSurfaceDestroyed(void *)
{
    Q_D(WaylandTextInput);

    d->focusDestroyListener.reset();

    d->focus = nullptr;
    d->focusResource = nullptr;
}

bool WaylandTextInput::isSurfaceEnabled(WaylandSurface *surface) const
{
    const Q_D(WaylandTextInput);

    return d->enabledSurfaces.values().contains(surface);
}

void WaylandTextInput::add(::wl_client *client, uint32_t id, int version)
{
    Q_D(WaylandTextInput);

    d->add(client, id, version);
}

const wl_interface *WaylandTextInput::interface()
{
    return WaylandTextInputPrivate::interface();
}

QByteArray WaylandTextInput::interfaceName()
{
    return WaylandTextInputPrivate::interfaceName();
}


void WaylandTextInput::sendModifiersMap(const QByteArray &modifiersMap)
{
    Q_D(WaylandTextInput);

    const QList<QByteArray> modifiers = modifiersMap.split('\0');

    int numModifiers = modifiers.size();
    if (modifiers.last().isEmpty())
        numModifiers--;

    for (int i = 0; i < numModifiers; ++i) {
        const auto modString = modifiers.at(i);
        if (modString == XKB_MOD_NAME_SHIFT)
            d->shiftModifierMask = 1 << i;
        else if (modString == XKB_MOD_NAME_CTRL)
            d->controlModifierMask = 1 << i;
        else if (modString == XKB_MOD_NAME_ALT)
            d->altModifierMask = 1 << i;
        else if (modString == XKB_MOD_NAME_LOGO)
            d->metaModifierMask = 1 << i;
        else
            qCDebug(gLcAuroraCompositorInputMethods) << "unsupported modifier name " << modString;
    }
    d->sendModifiersMap(modifiersMap);
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandtextinput.cpp"
