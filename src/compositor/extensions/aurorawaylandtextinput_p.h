// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDTEXTINPUT_P_H
#define AURORA_COMPOSITOR_WAYLANDTEXTINPUT_P_H

#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-text-input-unstable-v2.h>
#include <LiriAuroraCompositor/WaylandDestroyListener>

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QHash>
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

class WaylandTextInputClientState {
public:
    WaylandTextInputClientState();

    Qt::InputMethodQueries updatedQueries(const WaylandTextInputClientState &other) const;
    Qt::InputMethodQueries mergeChanged(const WaylandTextInputClientState &other);

    Qt::InputMethodHints hints = Qt::ImhNone;
    QRect cursorRectangle;
    QString surroundingText;
    int cursorPosition = 0;
    int anchorPosition = 0;
    QString preferredLanguage;

    Qt::InputMethodQueries changedState;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandTextInputPrivate : public WaylandCompositorExtensionPrivate, public PrivateServer::zwp_text_input_v2
{
    Q_DECLARE_PUBLIC(WaylandTextInput)
public:
    explicit WaylandTextInputPrivate(WaylandTextInput *self, WaylandCompositor *compositor);

    void sendInputMethodEvent(QInputMethodEvent *event);
    void sendKeyEvent(QKeyEvent *event);
    void sendInputPanelState();
    void sendTextDirection();
    void sendLocale();
    void sendModifiersMap(const QByteArray &modifiersMap);

    QVariant inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const;

    void setFocus(WaylandSurface *surface);

    WaylandCompositor *compositor = nullptr;

    WaylandSurface *focus = nullptr;
    Resource *focusResource = nullptr;
    WaylandDestroyListener focusDestroyListener;

    bool inputPanelVisible = false;

    std::unique_ptr<WaylandTextInputClientState> currentState;
    std::unique_ptr<WaylandTextInputClientState> pendingState;

    uint32_t serial = 0;

    QHash<Resource *, WaylandSurface*> enabledSurfaces;

protected:
    void zwp_text_input_v2_bind_resource(Resource *resource) override;
    void zwp_text_input_v2_destroy_resource(Resource *resource) override;

    void zwp_text_input_v2_destroy(Resource *resource) override;
    void zwp_text_input_v2_enable(Resource *resource, wl_resource *surface) override;
    void zwp_text_input_v2_disable(Resource *resource, wl_resource *surface) override;
    void zwp_text_input_v2_show_input_panel(Resource *resource) override;
    void zwp_text_input_v2_hide_input_panel(Resource *resource) override;
    void zwp_text_input_v2_set_surrounding_text(Resource *resource, const QString &text, int32_t cursor, int32_t anchor) override;
    void zwp_text_input_v2_set_content_type(Resource *resource, uint32_t hint, uint32_t purpose) override;
    void zwp_text_input_v2_set_cursor_rectangle(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void zwp_text_input_v2_set_preferred_language(Resource *resource, const QString &language) override;
    void zwp_text_input_v2_update_state(Resource *resource, uint32_t serial, uint32_t flags) override;

private:
    quint32 shiftModifierMask = 1;
    quint32 controlModifierMask = 2;
    quint32 altModifierMask = 4;
    quint32 metaModifierMask = 8;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDTEXTINPUT_P_H
