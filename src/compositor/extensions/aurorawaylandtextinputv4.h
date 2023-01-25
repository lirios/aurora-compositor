// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDTEXTINPUTV4_H
#define AURORA_COMPOSITOR_WAYLANDTEXTINPUTV4_H

#include <LiriAuroraCompositor/WaylandCompositorExtension>

struct wl_client;

class QInputMethodEvent;
class QKeyEvent;

namespace Aurora {

namespace Compositor {

class WaylandTextInputV4Private;

class WaylandSurface;

class WaylandTextInputV4 : public WaylandCompositorExtensionTemplate<WaylandTextInputV4>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandTextInputV4)
public:
    explicit WaylandTextInputV4(WaylandObject *container, WaylandCompositor *compositor);
    ~WaylandTextInputV4() override;

    void sendInputMethodEvent(QInputMethodEvent *event);
    void sendKeyEvent(QKeyEvent *event);

    QVariant inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const;

    WaylandSurface *focus() const;
    void setFocus(WaylandSurface *surface);

    bool isSurfaceEnabled(WaylandSurface *surface) const;

    void add(::wl_client *client, uint32_t id, int version);
    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void updateInputMethod(Qt::InputMethodQueries queries);
    void surfaceEnabled(WaylandSurface *surface);
    void surfaceDisabled(WaylandSurface *surface);

private:
    void focusSurfaceDestroyed(void *);
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDTEXTINPUTV4_H
