// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/aurorawaylandcompositorextension.h>

struct wl_client;

class QInputMethodEvent;
class QKeyEvent;

namespace Aurora {

namespace Compositor {

class WaylandSurface;
class WaylandQtTextInputMethodPrivate;

class WaylandQtTextInputMethod : public WaylandCompositorExtensionTemplate<WaylandQtTextInputMethod>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandQtTextInputMethod)
public:
    explicit WaylandQtTextInputMethod(WaylandObject *container, WaylandCompositor *compositor);
    ~WaylandQtTextInputMethod() override;

    WaylandSurface *focusedSurface() const;
    void setFocus(WaylandSurface *surface);

    bool isSurfaceEnabled(WaylandSurface *surface) const;

    void add(::wl_client *client, uint32_t id, int version);

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

    QVariant inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const;
    void sendInputMethodEvent(QInputMethodEvent *event);
    void sendKeyEvent(QKeyEvent *event);

Q_SIGNALS:
    void updateInputMethod(Qt::InputMethodQueries queries);
    void surfaceEnabled(Aurora::Compositor::WaylandSurface *surface);
    void surfaceDisabled(Aurora::Compositor::WaylandSurface *surface);


private Q_SLOTS:
    void sendVisibleChanged();
    void sendKeyboardRectangleChanged();
    void sendInputDirectionChanged();
    void sendLocaleChanged();
    void focusSurfaceDestroyed();
};

} // namespace Compositor

} // namespace Aurora

