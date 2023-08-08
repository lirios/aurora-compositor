// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QThread>

#include <LiriAuroraPlatform/DeviceIntegration>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/compositor.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/seat.h>
#include <KWayland/Client/subcompositor.h>
#include <KWayland/Client/shm_pool.h>
#include <KWayland/Client/xdgdecoration.h>
#include <KWayland/Client/xdgshell.h>

namespace Aurora {

namespace Platform {

class WaylandOutput;
class WaylandWindow;

class WaylandBackend : public QObject
{
    Q_OBJECT
public:
    explicit WaylandBackend(QObject *parent = nullptr);
    ~WaylandBackend();

    wl_display *display() const;

    void initialize();
    void destroy();

    void flush();

    KWayland::Client::Compositor *compositor() const;
    KWayland::Client::SubCompositor *subCompositor() const;
    KWayland::Client::ShmPool *shmPool() const;
    KWayland::Client::Seat *seat() const;
    KWayland::Client::XdgShell *xdgShell() const;
    KWayland::Client::XdgDecorationManager *xdgDecorationManager() const;

    Outputs outputs() const;

    WaylandWindow *findWindow(KWayland::Client::Surface *surface) const;
    WaylandOutput *findOutput(KWayland::Client::Surface *surface) const;

    QList<WaylandWindow *> windows() const;

    void registerWindow(WaylandWindow *window);
    void unregisterWindow(WaylandWindow *window);

    static WaylandBackend *instance();

Q_SIGNALS:
    void statusChanged(DeviceIntegration::Status status);
    void outputAdded(WaylandOutput *output);
    void outputRemoved(WaylandOutput *output);

private:
    QThread *m_connectionThread = nullptr;
    KWayland::Client::ConnectionThread *m_connectionThreadObject = nullptr;
    KWayland::Client::EventQueue *m_eventQueue = nullptr;
    KWayland::Client::Registry *m_registry = nullptr;
    KWayland::Client::Compositor *m_compositor = nullptr;
    KWayland::Client::SubCompositor *m_subCompositor = nullptr;
    KWayland::Client::ShmPool *m_shmPool = nullptr;
    KWayland::Client::Seat *m_seat = nullptr;
    KWayland::Client::XdgShell *m_xdgShell = nullptr;
    KWayland::Client::XdgDecorationManager *m_xdgDecorationManager = nullptr;

    int m_hostOutputsCount = 0;
    Outputs m_outputs;

    QList<WaylandWindow *> m_windows;

private slots:
    void handleOutputChanged();
    void destroyOutputs();
};

} // namespace Platform

} // namespace Aurora
