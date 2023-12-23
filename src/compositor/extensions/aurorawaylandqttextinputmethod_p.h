// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once


#include "aurorawaylandqttextinputmethod.h"

#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-qt-text-input-method-unstable-v1.h>
#include <LiriAuroraCompositor/aurorawaylanddestroylistener.h>

#include <QtCore/qrect.h>
#include <QtCore/qhash.h>

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

namespace Aurora {

namespace Compositor {

class WaylandCompositor;
class WaylandSurface;

class LIRIAURORACOMPOSITOR_EXPORT WaylandQtTextInputMethodPrivate : public WaylandCompositorExtensionPrivate, public PrivateServer::qt_text_input_method_v1
{
    Q_DECLARE_PUBLIC(WaylandQtTextInputMethod)
public:
    explicit WaylandQtTextInputMethodPrivate(WaylandCompositor *compositor);

    WaylandCompositor *compositor;
    WaylandSurface *focusedSurface = nullptr;
    Resource *resource = nullptr;
    QHash<Resource * , WaylandSurface *> enabledSurfaces;
    WaylandDestroyListener focusDestroyListener;
    bool inputPanelVisible = false;
    bool waitingForSync = false;

    Qt::InputMethodQueries updatingQueries;
    Qt::InputMethodHints hints;
    QString surroundingText;
    QString preferredLanguage;
    QRect cursorRectangle;
    int cursorPosition = 0;
    int anchorPosition = 0;
    int absolutePosition = 0;
    int surroundingTextOffset = 0;

private:
    void text_input_method_v1_enable(Resource *resource, struct ::wl_resource *surface) override;
    void text_input_method_v1_disable(Resource *resource, struct ::wl_resource *surface) override;
    void text_input_method_v1_destroy(Resource *resource) override;
    void text_input_method_v1_reset(Resource *resource) override;
    void text_input_method_v1_commit(Resource *resource) override;
    void text_input_method_v1_show_input_panel(Resource *resource) override;
    void text_input_method_v1_hide_input_panel(Resource *resource) override;
    void text_input_method_v1_update_hints(Resource *resource, int32_t hints) override;
    void text_input_method_v1_update_surrounding_text(Resource *resource, const QString &surroundingText, int32_t surroundingTextOffset) override;
    void text_input_method_v1_update_anchor_position(Resource *resource, int32_t anchorPosition) override;
    void text_input_method_v1_update_cursor_position(Resource *resource, int32_t cursorPosition) override;
    void text_input_method_v1_update_absolute_position(Resource *resource, int32_t absolutePosition) override;
    void text_input_method_v1_invoke_action(Resource *resource, int32_t type, int32_t cursorPosition) override;
    void text_input_method_v1_update_preferred_language(Resource *resource, const QString &preferredLanguage) override;
    void text_input_method_v1_update_cursor_rectangle(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void text_input_method_v1_start_update(Resource *resource, int32_t queries) override;
    void text_input_method_v1_end_update(Resource *resource) override;
    void text_input_method_v1_acknowledge_input_method(Resource *resource) override;
};

} // namespace Compositor

} // namespace Aurora

