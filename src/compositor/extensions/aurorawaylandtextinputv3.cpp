// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "aurorawaylandtextinputv3.h"
#include "aurorawaylandtextinputv3_p.h"

#include "aurorawaylandcompositor.h"
#include "aurorawaylandseat_p.h"

#include "aurorawaylandsurface.h"
#include "aurorawaylandview.h"
#include "aurorawaylandinputmethodeventbuilder_p.h"

#include <QGuiApplication>
#include <QInputMethodEvent>
#include <qpa/qwindowsysteminterface.h>

#if LIRI_FEATURE_aurora_xkbcommon
#include <LiriAuroraXkbCommonSupport/private/auroraxkbcommon_p.h>
#endif

namespace Aurora {

namespace Compositor {

Q_DECLARE_LOGGING_CATEGORY(gLcAuroraCompositorTextInput)

WaylandTextInputV3ClientState::WaylandTextInputV3ClientState()
{
}

Qt::InputMethodQueries WaylandTextInputV3ClientState::updatedQueries(const WaylandTextInputV3ClientState &other) const
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

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

    return queries;
}

Qt::InputMethodQueries WaylandTextInputV3ClientState::mergeChanged(const WaylandTextInputV3ClientState &other) {

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

    return queries;
}

WaylandTextInputV3Private::WaylandTextInputV3Private(WaylandCompositor *compositor)
    : compositor(compositor)
    , currentState(new WaylandTextInputV3ClientState)
    , pendingState(new WaylandTextInputV3ClientState)
{
}

void WaylandTextInputV3Private::sendInputMethodEvent(QInputMethodEvent *event)
{
    Q_Q(WaylandTextInputV3);
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    if (!focusResource || !focusResource->handle)
        return;

    bool needsDone = false;

    const QString &newPreeditString = event->preeditString();

    // Current cursor shape is only line. It means both cursorBegin
    // and cursorEnd will be the same values.
    int32_t preeditCursorPos = newPreeditString.length();

    if (event->replacementLength() > 0 || event->replacementStart() < 0) {
        if (event->replacementStart() <= 0 && (event->replacementLength() >= -event->replacementStart())) {
            const int selectionStart = qMin(currentState->cursorPosition, currentState->anchorPosition);
            const int selectionEnd = qMax(currentState->cursorPosition, currentState->anchorPosition);
            const int before = WaylandInputMethodEventBuilder::indexToWayland(currentState->surroundingText, -event->replacementStart(), selectionStart + event->replacementStart());
            const int after = WaylandInputMethodEventBuilder::indexToWayland(currentState->surroundingText, event->replacementLength() + event->replacementStart(), selectionEnd);
            send_delete_surrounding_text(focusResource->handle, before, after);
            needsDone = true;
        } else {
            qCWarning(gLcAuroraCompositorTextInput) << "Not yet supported case of replacement. Start:" << event->replacementStart() << "length:" << event->replacementLength();
        }
        preeditCursorPos = event->replacementStart() + event->replacementLength();
    }

    if (currentPreeditString != newPreeditString) {
        currentPreeditString = newPreeditString;
        send_preedit_string(focusResource->handle, currentPreeditString, preeditCursorPos, preeditCursorPos);
        needsDone = true;
    }
    if (!event->commitString().isEmpty()) {
        send_commit_string(focusResource->handle, event->commitString());
        needsDone = true;
    }

    if (needsDone)
        send_done(focusResource->handle, serial);
}


void WaylandTextInputV3Private::sendKeyEvent(QKeyEvent *event)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    Q_Q(WaylandTextInputV3);

    if (!focusResource || !focusResource->handle)
        return;

    send_commit_string(focusResource->handle, event->text());

    send_done(focusResource->handle, serial);
}

QVariant WaylandTextInputV3Private::inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO << property;

    switch (property) {
    case Qt::ImHints:
        return QVariant(static_cast<int>(currentState->hints));
    case Qt::ImCursorRectangle:
        return currentState->cursorRectangle;
    case Qt::ImFont:
        // Not supported
        return QVariant();
    case Qt::ImCursorPosition:
        qCDebug(gLcAuroraCompositorTextInput) << currentState->cursorPosition;
        return currentState->cursorPosition;
    case Qt::ImSurroundingText:
        qCDebug(gLcAuroraCompositorTextInput) << currentState->surroundingText;
        return currentState->surroundingText;
    case Qt::ImCurrentSelection:
        return currentState->surroundingText.mid(qMin(currentState->cursorPosition, currentState->anchorPosition),
                                                 qAbs(currentState->anchorPosition - currentState->cursorPosition));
    case Qt::ImMaximumTextLength:
        // Not supported
        return QVariant();
    case Qt::ImAnchorPosition:
        qCDebug(gLcAuroraCompositorTextInput) << currentState->anchorPosition;
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

    default:
        return QVariant();
    }
}

void WaylandTextInputV3Private::setFocus(WaylandSurface *surface)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;
    Q_Q(WaylandTextInputV3);

    if (focusResource && focus) {
        // sync before leave
        // IBUS commits by itself but qtvirtualkeyboard doesn't
        // And when handling chinese input, it is required to commit
        // before leaving the focus.
        if (qgetenv("QT_IM_MODULE") != QByteArrayLiteral("ibus")
                || qApp->inputMethod()->locale().language() == QLocale::Chinese) {
            qApp->inputMethod()->commit();
        }

        qApp->inputMethod()->hide();
        inputPanelVisible = false;
        send_leave(focusResource->handle, focus->resource());
        currentPreeditString.clear();
    }

    if (focus != surface)
        focusDestroyListener.reset();

    Resource *resource = surface ? resourceMap().value(surface->waylandClient()) : 0;
    if (resource && surface) {
        send_enter(resource->handle, surface->resource());

        if (focus != surface)
            focusDestroyListener.listenForDestruction(surface->resource());
    }

    focus = surface;
    focusResource = resource;
}

void WaylandTextInputV3Private::zwp_text_input_v3_bind_resource(Resource *resource)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    Q_UNUSED(resource);
}

void WaylandTextInputV3Private::zwp_text_input_v3_destroy_resource(Resource *resource)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    if (focusResource == resource)
        focusResource = nullptr;
}

void WaylandTextInputV3Private::zwp_text_input_v3_destroy(Resource *resource)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    wl_resource_destroy(resource->handle);
}

void WaylandTextInputV3Private::zwp_text_input_v3_enable(Resource *resource)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    Q_Q(WaylandTextInputV3);

    pendingState.reset(new WaylandTextInputV3ClientState);

    enabledSurfaces.insert(resource, focus);
    emit q->surfaceEnabled(focus);

    serial = 0;
    inputPanelVisible = true;
    qApp->inputMethod()->show();
}

void WaylandTextInputV3Private::zwp_text_input_v3_disable(PrivateServer::zwp_text_input_v3::Resource *resource)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    Q_Q(WaylandTextInputV3);

    WaylandSurface *s = enabledSurfaces.take(resource);
    emit q->surfaceDisabled(s);

    // When reselecting a word by setFocus
    if (qgetenv("QT_IM_MODULE") != QByteArrayLiteral("ibus")
            || qApp->inputMethod()->locale().language() == QLocale::Chinese) {
        qApp->inputMethod()->commit();
    }
    qApp->inputMethod()->reset();
    pendingState.reset(new WaylandTextInputV3ClientState);
}

void WaylandTextInputV3Private::zwp_text_input_v3_set_cursor_rectangle(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO << x << y << width << height;

    Q_Q(WaylandTextInputV3);

    if (resource != focusResource)
        return;

    pendingState->cursorRectangle = QRect(x, y, width, height);

    pendingState->changedState |= Qt::ImCursorRectangle;
}

void WaylandTextInputV3Private::zwp_text_input_v3_commit(Resource *resource)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    Q_Q(WaylandTextInputV3);

    if (resource != focusResource) {
        qCDebug(gLcAuroraCompositorTextInput) << "OBS: Disabled surface!!";
        return;
    }

    serial = serial < UINT_MAX ? serial + 1U : 0U;

    // Just increase serials and ignore empty commits
    if (!pendingState->changedState) {
        qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO << "pendingState is not changed";
        return;
    }

    // Selection starts.
    // But since qtvirtualkeyboard with hunspell does not reset its preedit string,
    // compositor forces to reset inputMethod.
    if ((currentState->cursorPosition == currentState->anchorPosition)
            && (pendingState->cursorPosition != pendingState->anchorPosition))
        qApp->inputMethod()->reset();

    // Enable reselection
    // This is a workaround to make qtvirtualkeyboad's state empty by clearing State::InputMethodClick.
    if (currentState->surroundingText == pendingState->surroundingText && currentState->cursorPosition != pendingState->cursorPosition)
        qApp->inputMethod()->invokeAction(QInputMethod::Click, pendingState->cursorPosition);

    Qt::InputMethodQueries queries = currentState->mergeChanged(*pendingState.data());
    pendingState.reset(new WaylandTextInputV3ClientState);

    if (queries) {
        qCDebug(gLcAuroraCompositorTextInput) << "QInputMethod::update() after commit with" << queries;

        qApp->inputMethod()->update(queries);
    }
}

void WaylandTextInputV3Private::zwp_text_input_v3_set_content_type(Resource *resource, uint32_t hint, uint32_t purpose)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO << hint << purpose;

    if (resource != focusResource)
        return;

    pendingState->hints = Qt::ImhNone;

    if ((hint & content_hint_completion) == 0)
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

    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO << pendingState->hints;

    pendingState->changedState |= Qt::ImHints;
}

void WaylandTextInputV3Private::zwp_text_input_v3_set_surrounding_text(Resource *resource, const QString &text, int32_t cursor, int32_t anchor)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO << text << cursor << anchor;

    if (resource != focusResource)
        return;

    pendingState->surroundingText = text;
    pendingState->cursorPosition = WaylandInputMethodEventBuilder::indexFromWayland(text, cursor);
    pendingState->anchorPosition = WaylandInputMethodEventBuilder::indexFromWayland(text, anchor);

    pendingState->changedState |= Qt::ImSurroundingText | Qt::ImCursorPosition | Qt::ImAnchorPosition;
}

void WaylandTextInputV3Private::zwp_text_input_v3_set_text_change_cause(Resource *resource, uint32_t cause)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    Q_UNUSED(resource);
    Q_UNUSED(cause);
}

WaylandTextInputV3::WaylandTextInputV3(WaylandObject *container, WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate(container, *new WaylandTextInputV3Private(compositor))
{
    connect(&d_func()->focusDestroyListener, &WaylandDestroyListener::fired,
            this, &WaylandTextInputV3::focusSurfaceDestroyed);
}

WaylandTextInputV3::~WaylandTextInputV3()
{
}

void WaylandTextInputV3::sendInputMethodEvent(QInputMethodEvent *event)
{
    Q_D(WaylandTextInputV3);

    d->sendInputMethodEvent(event);
}

void WaylandTextInputV3::sendKeyEvent(QKeyEvent *event)
{
    Q_D(WaylandTextInputV3);

    d->sendKeyEvent(event);
}

QVariant WaylandTextInputV3::inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const
{
    const Q_D(WaylandTextInputV3);

    return d->inputMethodQuery(property, argument);
}

WaylandSurface *WaylandTextInputV3::focus() const
{
    const Q_D(WaylandTextInputV3);

    return d->focus;
}

void WaylandTextInputV3::setFocus(WaylandSurface *surface)
{
    Q_D(WaylandTextInputV3);

    d->setFocus(surface);
}

void WaylandTextInputV3::focusSurfaceDestroyed(void *)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    Q_D(WaylandTextInputV3);

    d->focusDestroyListener.reset();

    d->focus = nullptr;
    d->focusResource = nullptr;
}

bool WaylandTextInputV3::isSurfaceEnabled(WaylandSurface *surface) const
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    const Q_D(WaylandTextInputV3);

    return d->enabledSurfaces.values().contains(surface);
}

void WaylandTextInputV3::add(::wl_client *client, uint32_t id, int version)
{
    qCDebug(gLcAuroraCompositorTextInput) << Q_FUNC_INFO;

    Q_D(WaylandTextInputV3);

    d->add(client, id, version);
}

const wl_interface *WaylandTextInputV3::interface()
{
    return WaylandTextInputV3Private::interface();
}

QByteArray WaylandTextInputV3::interfaceName()
{
    return WaylandTextInputV3Private::interfaceName();
}

} // namespace Compositor

} // namespace Aurora
