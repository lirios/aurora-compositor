// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KWayland/Client/region.h>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/xdgshell.h>

#include "waylandbackend.h"
#include "waylandloggingcategories.h"
#include "waylandoutput.h"
#include "waylandwindow.h"

using namespace KWayland::Client;

namespace Aurora {

namespace Platform {

Q_GLOBAL_STATIC(WaylandBackend, gWaylandBackend)

WaylandBackend::WaylandBackend(QObject *parent)
    : QObject(parent)
    , m_connectionThread(new QThread())
    , m_connectionThreadObject(new KWayland::Client::ConnectionThread())
    , m_eventQueue(new EventQueue(this))
    , m_registry(new Registry(this))
{
    m_connectionThreadObject->moveToThread(m_connectionThread);
}

WaylandBackend::~WaylandBackend()
{
    if (m_connectionThreadObject) {
        m_connectionThreadObject->flush();
        m_connectionThreadObject->deleteLater();
        m_connectionThreadObject = nullptr;
    }

    if (m_connectionThread) {
        m_connectionThread->quit();
        m_connectionThread->deleteLater();
        m_connectionThread = nullptr;
    }
}

wl_display *WaylandBackend::display() const
{
    if (m_connectionThreadObject)
        return m_connectionThreadObject->display();
    return nullptr;
}

void WaylandBackend::initialize()
{
    // Compositor
    connect(m_registry, &Registry::compositorAnnounced, this,
            [this](quint32 name, quint32 version) {
                if (version < 4) {
                    qCCritical(gLcWayland, "wl_compositor version 4 or later is required");
                    qFatal("Aborting...");
                }
                m_compositor = m_registry->createCompositor(name, version);
            });

    // Compositor for sub-surfaces
    connect(m_registry, &Registry::subCompositorAnnounced, this,
            [this](quint32 name, quint32 version) {
                m_subCompositor = m_registry->createSubCompositor(name, version);
            });

    // Shared memory pool
    connect(m_registry, &Registry::shmAnnounced, this, [this](quint32 name, quint32 version) {
        m_shmPool = m_registry->createShmPool(name, version);
    });

    // Seat
    connect(m_registry, &Registry::seatAnnounced, this, [this](quint32 name, quint32 version) {
        m_seat = m_registry->createSeat(name, version);
    });

    // Output
    connect(m_registry, &Registry::outputAnnounced, this, [this](quint32 name, quint32 version) {
        m_hostOutputsCount++;

        auto *hostOutput = m_registry->createOutput(name, version);

        connect(hostOutput, &KWayland::Client::Output::changed, this,
                &WaylandBackend::handleOutputChanged);
    });

    // XDG shell
    connect(m_registry, &Registry::xdgShellStableAnnounced, this,
            [this](quint32 name, quint32 version) {
                m_xdgShell = m_registry->createXdgShell(name, version);
            });

    // XDG decoration
    connect(m_registry, &Registry::xdgDecorationAnnounced, this,
            [this](quint32 name, quint32 version) {
                m_xdgDecorationManager = m_registry->createXdgDecorationManager(name, version);
            });

    // Verify we have what we need
    connect(m_registry, &Registry::interfacesAnnounced, this, [this]() {
        if (!m_xdgShell || !m_xdgShell->isValid())
            qCritical("The xdg_wm_base interface was not found, cannot continue!");
    });

    // Connection
    connect(
            m_connectionThreadObject, &ConnectionThread::connected, this,
            [this]() {
                qCDebug(gLcWayland) << "Connected to:" << m_connectionThreadObject->socketName();

                // Create the event queue
                m_eventQueue->setup(m_connectionThreadObject);
                m_registry->setEventQueue(m_eventQueue);

                // Registry
                m_registry->create(m_connectionThreadObject);
                m_registry->setup();
                m_connectionThreadObject->flush();
            },
            Qt::QueuedConnection);
    connect(
            m_connectionThreadObject, &ConnectionThread::connectionDied, this,
            [this]() {
                Q_EMIT statusChanged(DeviceIntegration::Status::Failed);

                destroyOutputs();

                if (m_seat)
                    m_seat->destroy();

                if (m_shmPool)
                    m_shmPool->destroy();

                if (m_xdgDecorationManager)
                    m_xdgDecorationManager->destroy();
                if (m_xdgShell)
                    m_xdgShell->destroy();

                if (m_subCompositor)
                    m_subCompositor->destroy();
                if (m_compositor)
                    m_compositor->destroy();
                if (m_registry)
                    m_registry->destroy();
                if (m_eventQueue)
                    m_eventQueue->destroy();
            },
            Qt::QueuedConnection);
    connect(m_connectionThreadObject, &ConnectionThread::failed, this, [this]() {
        qCDebug(gLcWayland) << "Failed to connect to:" << m_connectionThreadObject->socketName();

        Q_EMIT statusChanged(DeviceIntegration::Status::Failed);

        destroyOutputs();
    });

    // Connect
    m_connectionThread->start();
    m_connectionThreadObject->initConnection();
}

void WaylandBackend::destroy()
{
    destroyOutputs();

    if (m_xdgDecorationManager) {
        m_xdgDecorationManager->release();
        m_xdgDecorationManager->destroy();
    }
    if (m_xdgShell) {
        m_xdgShell->release();
        m_xdgShell->destroy();
    }

    if (m_subCompositor) {
        m_subCompositor->release();
        m_subCompositor->destroy();
    }
    if (m_seat) {
        m_seat->release();
        m_seat->destroy();
    }
    if (m_shmPool) {
        m_shmPool->release();
        m_shmPool->destroy();
    }
    if (m_compositor) {
        m_compositor->release();
        m_compositor->destroy();
    }
    if (m_registry) {
        m_registry->release();
        m_registry->destroy();
    }

    if (m_connectionThread) {
        m_connectionThread->quit();
        m_connectionThread->wait();
    }

    if (m_eventQueue)
        m_eventQueue->release();
}

void WaylandBackend::flush()
{
    if (m_connectionThreadObject)
        m_connectionThreadObject->flush();
}

WaylandBackend *WaylandBackend::instance()
{
    return gWaylandBackend();
}

KWayland::Client::Compositor *WaylandBackend::compositor() const
{
    return m_compositor;
}

KWayland::Client::SubCompositor *WaylandBackend::subCompositor() const
{
    return m_subCompositor;
}

KWayland::Client::ShmPool *WaylandBackend::shmPool() const
{
    return m_shmPool;
}

KWayland::Client::Seat *WaylandBackend::seat() const
{
    return m_seat;
}

KWayland::Client::XdgShell *WaylandBackend::xdgShell() const
{
    return m_xdgShell;
}

KWayland::Client::XdgDecorationManager *WaylandBackend::xdgDecorationManager() const
{
    return m_xdgDecorationManager;
}

Outputs WaylandBackend::outputs() const
{
    return m_outputs;
}

WaylandWindow *WaylandBackend::findWindow(KWayland::Client::Surface *surface) const
{
    for (auto *wlWindow : qAsConst(m_windows)) {
        if (wlWindow->surface() == surface)
            return wlWindow;
    }

    return nullptr;
}

WaylandOutput *WaylandBackend::findOutput(KWayland::Client::Surface *surface) const
{
    if (auto *wlWindow = findWindow(surface))
        return static_cast<WaylandOutput *>(wlWindow->output());

    return nullptr;
}

QList<WaylandWindow *> WaylandBackend::windows() const
{
    return m_windows;
}

void WaylandBackend::registerWindow(WaylandWindow *window)
{
    m_windows.append(window);
}

void WaylandBackend::unregisterWindow(WaylandWindow *window)
{
    m_windows.removeOne(window);
}

void WaylandBackend::handleOutputChanged()
{
    auto *hostOutput = static_cast<KWayland::Client::Output *>(sender());
    if (!hostOutput)
        return;

    const auto index = m_outputs.size() + 1;
    auto *output = new WaylandOutput(hostOutput, QStringLiteral("WL-%1").arg(index));
    m_outputs.append(output);
    Q_EMIT outputAdded(output);

    if (m_outputs.size() == m_hostOutputsCount)
        Q_EMIT statusChanged(DeviceIntegration::Status::Ready);
}

void WaylandBackend::destroyOutputs()
{
    auto it = m_outputs.begin();
    while (it != m_outputs.end()) {
        auto *output = static_cast<WaylandOutput *>(*it);
        Q_EMIT outputRemoved(output);
        output->destroy();
        output->deleteLater();
        it = m_outputs.erase(it);
    }
}

} // namespace Platform

} // namespace Aurora
