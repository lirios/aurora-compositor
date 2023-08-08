// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtMath>

#include <KWayland/Client/compositor.h>

#include "waylandbackend.h"
#include "waylandcursor.h"
#include "waylandwindow.h"

namespace Aurora {

namespace Platform {

WaylandCursor::WaylandCursor(WaylandWindow *window, QObject *parent)
    : QObject(parent)
    , m_window(window)
    , m_surface(WaylandBackend::instance()->compositor()->createSurface(this))
{
}

WaylandCursor::~WaylandCursor()
{
    if (m_surface) {
        m_surface->release();
        m_surface->destroy();
        m_surface = nullptr;
    }
}

bool WaylandCursor::isEnabled() const
{
    return m_enabled;
}

void WaylandCursor::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    render();
}

KWayland::Client::Pointer *WaylandCursor::pointer() const
{
    return m_pointer;
}

void WaylandCursor::setPointer(KWayland::Client::Pointer *pointer)
{
    if (m_pointer == pointer)
        return;

    m_pointer = pointer;
    if (m_pointer)
        m_pointer->setCursor(m_surface, m_hotSpot);
}

void WaylandCursor::update(const QImage &image, const QPoint &hotSpot, qreal scale)
{
    if (m_image != image || m_scale != scale || m_hotSpot != hotSpot) {
        m_image = image;
        m_buffer = WaylandBackend::instance()->shmPool()->createBuffer(m_image);
        m_scale = scale;
        m_hotSpot = hotSpot;
        render();
    }
}

void WaylandCursor::render()
{
    if (m_enabled) {
        m_surface->attachBuffer(m_buffer);
        m_surface->setScale(qCeil(m_scale));
        m_surface->damageBuffer(QRect(0, 0, INT32_MAX, INT32_MAX));
    } else {
        m_surface->attachBuffer(KWayland::Client::Buffer::Ptr());
    }
    m_surface->commit(KWayland::Client::Surface::CommitFlag::None);

    m_window->present();
}

} // namespace Platform

} // namespace Aurora