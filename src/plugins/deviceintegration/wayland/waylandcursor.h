// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KWayland/Client/pointer.h>
#include <KWayland/Client/surface.h>

namespace Aurora {

namespace Platform {

class WaylandWindow;

class WaylandCursor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled NOTIFY enabledChanged)
public:
    explicit WaylandCursor(WaylandWindow *window, QObject *parent = nullptr);
    ~WaylandCursor();

    bool isEnabled() const;
    void setEnabled(bool enabled);

    KWayland::Client::Pointer *pointer() const;
    void setPointer(KWayland::Client::Pointer *pointer);

    void update(const QImage &image, const QPoint &hotSpot, qreal scale);

Q_SIGNALS:
    void enabledChanged(bool enabled);

private:
    bool m_enabled = false;
    QPointer<WaylandWindow> m_window;
    KWayland::Client::Surface *m_surface = nullptr;
    KWayland::Client::Pointer *m_pointer = nullptr;
    QImage m_image;
    KWayland::Client::Buffer::Ptr m_buffer;
    QPoint m_hotSpot;
    qreal m_scale = 1;

    void render();
};

} // namespace Platform

} // namespace Aurora
