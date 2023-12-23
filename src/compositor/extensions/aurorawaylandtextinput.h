// Copyright (C) 2017-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandCompositorExtension>

struct wl_client;

class QInputMethodEvent;
class QKeyEvent;

namespace Aurora {

namespace Compositor {

class WaylandTextInputPrivate;

class WaylandSurface;

class WaylandTextInput : public WaylandCompositorExtensionTemplate<WaylandTextInput>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandTextInput)
public:
    explicit WaylandTextInput(WaylandObject *container, WaylandCompositor *compositor);
    ~WaylandTextInput() override;

    void sendInputMethodEvent(QInputMethodEvent *event);
    void sendKeyEvent(QKeyEvent *event);

    QVariant inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const;

    WaylandSurface *focus() const;
    void setFocus(WaylandSurface *surface);

    bool isSurfaceEnabled(WaylandSurface *surface) const;

    void add(::wl_client *client, uint32_t id, int version);
    static const struct wl_interface *interface();
    static QByteArray interfaceName();

    void sendModifiersMap(const QByteArray &modifiersMap);

Q_SIGNALS:
    void updateInputMethod(Qt::InputMethodQueries queries);
    void surfaceEnabled(Aurora::Compositor::WaylandSurface *surface);
    void surfaceDisabled(Aurora::Compositor::WaylandSurface *surface);

private:
    QScopedPointer<WaylandTextInputPrivate> const d_ptr;

    void focusSurfaceDestroyed(void *);
    void sendInputPanelState();
    void sendTextDirection();
    void sendLocale();
};

} // namespace Compositor

} // namespace Aurora

