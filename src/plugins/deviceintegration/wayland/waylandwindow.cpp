// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QScreen>

#include "waylandbackend.h"
#include "waylandcursor.h"
#include "waylandloggingcategories.h"
#include "waylandoutput.h"
#include "waylandwindow.h"

enum ResourceType {
    WlSurfaceResourceType
};

static int resourceType(const QByteArray &name)
{
    static const QByteArray names[] = { QByteArrayLiteral("wl_surface") };
    const int numResourceTypes = sizeof(names) / sizeof(names[0]);

    for (int i = 0; i < numResourceTypes; ++i) {
        if (name.toLower() == names[i])
            return i;
    }

    return -1;
}

namespace Aurora {

namespace Platform {

WaylandWindow::WaylandWindow(Output *output, QWindow *qtWindow, QObject *parent)
    : Window(output, qtWindow, parent)
    , m_output(static_cast<WaylandOutput *>(output))
    , m_cursor(new WaylandCursor(this, this))
    , m_shapeCursorSource(new Core::ShapeCursorSource(this))
{
    WaylandBackend::instance()->registerWindow(this);
}

WaylandWindow::~WaylandWindow()
{
    WaylandBackend::instance()->unregisterWindow(this);
    destroy();
}

WaylandCursor *WaylandWindow::cursor() const
{
    return m_cursor;
}

KWayland::Client::Surface *WaylandWindow::surface() const
{
    return m_surface;
}

void *WaylandWindow::resource(const QByteArray &name)
{
    void *result = nullptr;

    switch (resourceType(name)) {
    case WlSurfaceResourceType:
        if (m_surface)
            result = m_surface->operator wl_surface *();
        break;
    default:
        break;
    }

    return result;
}

bool WaylandWindow::create()
{
    // Do not create twice
    if (m_surface || m_xdgToplevel)
        return true;

    const auto geometry = qtWindow()->screen()->geometry();
    const auto size = geometry.size();

    // Create a surface
    m_surface = WaylandBackend::instance()->compositor()->createSurface(this);
    m_surface->setSize(size);
    m_surface->setOpaqueRegion(
            WaylandBackend::instance()->compositor()->createRegion(QRegion(geometry)).get());
    m_surface->commit(KWayland::Client::Surface::CommitFlag::None);

    // Create a toplevel window
    m_xdgToplevel = WaylandBackend::instance()->xdgShell()->createSurface(m_surface, this);
    connect(m_xdgToplevel, &KWayland::Client::XdgShellSurface::configureRequested, this,
            [this](const QSize &, KWayland::Client::XdgShellSurface::States, quint32 serial) {
                m_xdgToplevel->ackConfigure(serial);
            });
    m_xdgToplevel->setTitle(QStringLiteral("Aurora Compositor"));
    m_xdgToplevel->setMinSize(size);
    m_xdgToplevel->setMaxSize(size);

    // We don't want the compositor to draw any decoration for us
    if (WaylandBackend::instance()->xdgDecorationManager()) {
        m_xdgDecoration = WaylandBackend::instance()->xdgDecorationManager()->getToplevelDecoration(
                m_xdgToplevel, this);
        m_xdgDecoration->setMode(KWayland::Client::XdgDecoration::Mode::ClientSide);
    }

    // Load X cursor theme
    connect(m_shapeCursorSource, &Core::CursorSource::changed, this, [this] {
        m_cursor->update(m_shapeCursorSource->image(), m_shapeCursorSource->hotSpot().toPoint(),
                         m_output->scale());
    });
    m_shapeCursorSource->loadTheme(QStringLiteral("default"), 32, m_output->scale());

    // Set default cursor
    QCursor cursor(Qt::ArrowCursor);
    changeCursor(&cursor);

    return true;
}

void WaylandWindow::destroy()
{
    if (m_destroyed)
        return;

    m_destroyed = true;

    qCDebug(gLcWayland) << "Window for" << qtWindow() << "is about to be destroyed...";

    if (m_xdgDecoration) {
        m_xdgDecoration->release();
        m_xdgDecoration->destroy();
        m_xdgDecoration = nullptr;
    }

    if (m_xdgToplevel) {
        m_xdgToplevel->release();
        m_xdgToplevel->destroy();
        m_xdgToplevel = nullptr;
    }

    if (m_surface) {
        m_surface->release();
        m_surface->destroy();
        m_surface = nullptr;
    }
}

void WaylandWindow::changeCursor(QCursor *cursor)
{
    const Qt::CursorShape newShape = cursor ? cursor->shape() : Qt::ArrowCursor;
    if (m_cursorShape == newShape && newShape != Qt::BitmapCursor)
        return;

    if (m_cursorShape == Qt::BitmapCursor)
        m_cursorImage = QImage();

    m_cursorShape = newShape;
    if (newShape == Qt::BitmapCursor) {
        // Use the custom cursor image
        m_cursor->update(cursor->pixmap().toImage(), cursor->hotSpot(), m_output->scale());
    } else {
        // Load the image from the cursor theme
        m_shapeCursorSource->setShape(newShape);
    }
}

void WaylandWindow::present()
{
    m_surface->commit(KWayland::Client::Surface::CommitFlag::None);
}

} // namespace Platform

} // namespace Aurora
