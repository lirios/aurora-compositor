// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-text-input-unstable-v4-wip.h>
#include <LiriAuroraCompositor/WaylandDestroyListener>

#include <QtCore/QObject>
#include <QtCore/QRect>
#include <QtGui/QInputMethod>
#include <LiriAuroraCompositor/WaylandSurface>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Aurora API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class QInputMethodEvent;
class QKeyEvent;

namespace Aurora {

namespace Compositor {

class WaylandCompositor;
class WaylandView;

class WaylandTextInputV4ClientState {
public:
    WaylandTextInputV4ClientState();

    Qt::InputMethodQueries updatedQueries(const WaylandTextInputV4ClientState &other) const;
    Qt::InputMethodQueries mergeChanged(const WaylandTextInputV4ClientState &other);

    Qt::InputMethodHints hints = Qt::ImhNone;
    QRect cursorRectangle;
    QString surroundingText;
    int cursorPosition = 0;
    int anchorPosition = 0;

    Qt::InputMethodQueries changedState;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandTextInputV4Private : public WaylandCompositorExtensionPrivate, public PrivateServer::zwp_text_input_v4
{
    Q_DECLARE_PUBLIC(WaylandTextInputV4)
public:
    explicit WaylandTextInputV4Private(WaylandCompositor *compositor);

    void sendInputMethodEvent(QInputMethodEvent *event);
    void sendKeyEvent(QKeyEvent *event);

    QVariant inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const;

    void setFocus(WaylandSurface *surface);

    WaylandCompositor *compositor = nullptr;

    WaylandSurface *focus = nullptr;
    Resource *focusResource = nullptr;
    WaylandDestroyListener focusDestroyListener;

    bool inputPanelVisible = false;

    QString currentPreeditString;

    QScopedPointer<WaylandTextInputV4ClientState> currentState;
    QScopedPointer<WaylandTextInputV4ClientState> pendingState;

    uint32_t serial = 0;

    QHash<Resource *, WaylandSurface*> enabledSurfaces;

protected:
    void zwp_text_input_v4_bind_resource(Resource *resource) override;
    void zwp_text_input_v4_destroy_resource(Resource *resource) override;

    void zwp_text_input_v4_destroy(Resource *resource) override;
    void zwp_text_input_v4_enable(Resource *resource) override;
    void zwp_text_input_v4_disable(Resource *resource) override;
    void zwp_text_input_v4_set_surrounding_text(Resource *resource, const QString &text, int32_t cursor, int32_t anchor) override;
    void zwp_text_input_v4_set_text_change_cause(Resource *resource, uint32_t cause) override;
    void zwp_text_input_v4_set_content_type(Resource *resource, uint32_t hint, uint32_t purpose) override;
    void zwp_text_input_v4_set_cursor_rectangle(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void zwp_text_input_v4_commit(Resource *resource) override;
};

} // namespace Compositor

} // namespace Aurora

