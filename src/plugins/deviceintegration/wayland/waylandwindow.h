// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <LiriAuroraPlatform/Window>
#include <LiriAuroraCore/ShapeCursorSource>

#include <KWayland/Client/xdgdecoration.h>
#include <KWayland/Client/xdgshell.h>
#include <KWayland/Client/region.h>
#include <KWayland/Client/surface.h>

struct wl_egl_window;

namespace Aurora {

namespace Platform {

class WaylandCursor;

class WaylandWindow : public Window
{
    Q_OBJECT
public:
    explicit WaylandWindow(Output *output, QWindow *window, QObject *parent = nullptr);
    ~WaylandWindow();

    WaylandCursor *cursor() const;

    KWayland::Client::Surface *surface() const;

    void *resource(const QByteArray &name) override;

    bool create() override;
    void destroy() override;

    void changeCursor(QCursor *cursor) override;

    void present();

private:
    bool m_destroyed = false;
    WaylandOutput *m_output = nullptr;
    WaylandCursor *m_cursor = nullptr;
    KWayland::Client::Surface *m_surface = nullptr;
    KWayland::Client::XdgShellSurface *m_xdgToplevel = nullptr;
    KWayland::Client::XdgDecoration *m_xdgDecoration = nullptr;

    Core::ShapeCursorSource *m_shapeCursorSource = nullptr;
    Qt::CursorShape m_cursorShape = Qt::BlankCursor;
    QImage m_cursorImage;
};

} // namespace Platform

} // namespace Aurora
