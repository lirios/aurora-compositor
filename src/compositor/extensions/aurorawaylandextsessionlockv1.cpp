// SPDX-FileCopyrightText: 2022-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aurorawaylandcompositor.h"
#include "aurorawaylandextsessionlockv1_p.h"
#include "aurorawaylandextsessionlockv1integration_p.h"
#include "aurorawaylandsurface_p.h"
#include "aurorawaylandquickshellintegration.h"
#include "aurorawaylandquickshellsurfaceitem.h"
#include "aurorawaylandseat_p.h"

#include <wayland-server-protocol.h>

namespace Aurora {

namespace Compositor {

/*
 * WaylandExtSessionLockManagerV1
 */

WaylandExtSessionLockManagerV1::WaylandExtSessionLockManagerV1()
    : WaylandShellTemplate<WaylandExtSessionLockManagerV1>()
{
}

WaylandExtSessionLockManagerV1::WaylandExtSessionLockManagerV1(WaylandCompositor *compositor)
    : WaylandShellTemplate<WaylandExtSessionLockManagerV1>(compositor)
{
}

bool WaylandExtSessionLockManagerV1::isLocked() const
{
    Q_D(const WaylandExtSessionLockManagerV1);
    return d->m_locked;
}

bool WaylandExtSessionLockManagerV1::hasClientConnected() const
{
    Q_D(const WaylandExtSessionLockManagerV1);
    return d->m_clientConnected;
}

void WaylandExtSessionLockManagerV1::initialize()
{
    Q_D(WaylandExtSessionLockManagerV1);

    WaylandShellTemplate::initialize();
    auto *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qCWarning(gLcAuroraCompositorExtSessionLockV1) << "Failed to find WaylandCompositor when initializing WaylandExtSessionLockManagerV1";
        return;
    }
    d->init(compositor->display(), WaylandExtSessionLockManagerV1Private::interfaceVersion());
}

const wl_interface *WaylandExtSessionLockManagerV1::interface()
{
    return WaylandExtSessionLockManagerV1Private::interface();
}

QByteArray WaylandExtSessionLockManagerV1::interfaceName()
{
    return WaylandExtSessionLockManagerV1Private::interfaceName();
}

/*
 * WaylandExtSessionLockManagerV1Private
 */

WaylandExtSessionLockManagerV1Private::WaylandExtSessionLockManagerV1Private()
{
}

bool WaylandExtSessionLockManagerV1Private::isLocked() const
{
    return m_locked;
}

void WaylandExtSessionLockManagerV1Private::setLocked(bool value)
{
    Q_Q(WaylandExtSessionLockManagerV1);

    if (m_locked != value) {
        m_locked = value;
        emit q->lockedChanged();
    }

    if (!m_locked) {
        // Allow new clients to bind
        sessionLock.clear();
        m_client = nullptr;
    }

    auto *compositor = static_cast<WaylandCompositor *>(q->extensionContainer());
    const auto seats = compositor->seats();
    for (auto *seat : seats) {
        // When locked, allow input only to the surfaces of this client
        WaylandSeatPrivate::get(seat)->setExclusiveClient(m_locked ? m_client->client() : nullptr);

        // When locked, save the old focused surface because we will give focus
        // only to lock surfaces
        if (m_locked) {
            if (seat->keyboardFocus())
                m_oldFocus[seat] = seat->keyboardFocus();
        } else {
            if (auto *item = qobject_cast<WaylandQuickItem *>(m_oldFocus[seat]->primaryView()->renderObject()))
                item->takeFocus(seat);
            else
                seat->setKeyboardFocus(m_oldFocus[seat]);
            m_oldFocus.remove(seat);
        }
    }
}

void WaylandExtSessionLockManagerV1Private::setClientConnected(bool value)
{
    Q_Q(WaylandExtSessionLockManagerV1);

    if (m_clientConnected != value) {
        m_clientConnected = value;
        emit q->hasClientConnectedChanged();
    }
}

QList<WaylandOutput *> WaylandExtSessionLockManagerV1Private::outputs() const
{
    return m_outputs;
}

void WaylandExtSessionLockManagerV1Private::registerLockSurface(WaylandExtSessionLockSurfaceV1 *lockSurface)
{
    m_lockSurfaces.insert(lockSurface->surface()->client()->client(), lockSurface);
    m_outputs.append(lockSurface->output());
}

void WaylandExtSessionLockManagerV1Private::unregisterLockSurface(WaylandExtSessionLockSurfaceV1 *lockSurface)
{
    auto *lockSurfacePrivate = WaylandExtSessionLockSurfaceV1Private::get(lockSurface);
    if (m_lockSurfaces.remove(lockSurfacePrivate->resource()->client(), lockSurface))
        m_outputs.removeOne(lockSurface->output());
    else
        qCWarning(gLcAuroraCompositorExtSessionLockV1,
                  "%s Unexpected state. Can't find registered lock surface\n", Q_FUNC_INFO);
}

void WaylandExtSessionLockManagerV1Private::ext_session_lock_manager_v1_bind_resource(Resource *resource)
{
    // We can allow only one client at a time
    if (m_client) {
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_IMPLEMENTATION,
                               "client can bind only once");
        return;
    }

    // Client connected
    setClientConnected(true);
}

void WaylandExtSessionLockManagerV1Private::ext_session_lock_manager_v1_destroy_resource(Resource *resource)
{
    // Client no longer connected
    setClientConnected(false);
}

void WaylandExtSessionLockManagerV1Private::ext_session_lock_manager_v1_destroy(Resource *resource)
{
    if (!sessionLock.isNull())
        sessionLock.data()->deleteLater();

    wl_resource_destroy(resource->handle);

    // Allow new clients to bind
    m_client = nullptr;
}

void WaylandExtSessionLockManagerV1Private::ext_session_lock_manager_v1_lock(Resource *resource, uint32_t id)
{
    Q_Q(WaylandExtSessionLockManagerV1);

    WaylandResource lockResource(wl_resource_create(resource->client(),
                                                    &ext_session_lock_v1_interface,
                                                    wl_resource_get_version(resource->handle), id));

    auto *lock = new WaylandExtSessionLockV1(q, lockResource);

    if (sessionLock.isNull()) {
        // No previous lock was acquired, we can proceed
        sessionLock = lock;
        lock->send_locked();

        // Save the client to be used later as the exclusive client on seats
        auto *compositor = static_cast<WaylandCompositor *>(q->extensionContainer());
        m_client = WaylandClient::fromWlClient(compositor, resource->client());

        // Lock the session: remember that once it's locked it will stay locked
        // until the unlock_and_destroy event is triggered
        setLocked(true);
    } else {
        // A lock was previously acquired and we can only allow one client at a time,
        // refuse to lock by sending the finished event, but the user will still
        // see the session locked for security reasons (maybe the client was killed
        // or it just crashed and the compositor is about to restart it)
        lock->send_finished();
    }
}

/*
 * WaylandExtSessionLockV1
 */

WaylandExtSessionLockV1::WaylandExtSessionLockV1(WaylandExtSessionLockManagerV1 *manager, const WaylandResource &resource)
    : WaylandShellTemplate<WaylandExtSessionLockV1>(manager)
{
    initialize();
    init(resource.resource());
}

WaylandExtSessionLockV1::~WaylandExtSessionLockV1()
{
}

void WaylandExtSessionLockV1::ext_session_lock_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

void WaylandExtSessionLockV1::ext_session_lock_v1_destroy(Resource *resource)
{
    auto *manager = qobject_cast<WaylandExtSessionLockManagerV1 *>(extensionContainer());
    Q_ASSERT(manager);

    const bool isLocked = !WaylandExtSessionLockManagerV1Private::get(manager)->sessionLock.isNull();
    if (isLocked) {
        wl_resource_post_error(resource->handle, error_invalid_destroy,
                               "attempted to destroy session lock while locked");
        return;
    }

    wl_resource_destroy(resource->handle);
}

void WaylandExtSessionLockV1::ext_session_lock_v1_get_lock_surface(Resource *resource, uint32_t id, wl_resource *surface, wl_resource *output)
{
    auto *manager = qobject_cast<WaylandExtSessionLockManagerV1 *>(extensionContainer());
    Q_ASSERT(manager);

    auto *managerPrivate = WaylandExtSessionLockManagerV1Private::get(manager);
    Q_ASSERT(managerPrivate);

    auto *wlSurface = WaylandSurface::fromResource(surface);
    if (!wlSurface) {
        qCWarning(gLcAuroraCompositorExtSessionLockV1, "Resource wl_surface@%d doesn't exist",
                  wl_resource_get_id(surface));
        return;
    }

    if (wlSurface->hasContent()) {
        wl_resource_post_error(resource->handle, error_already_constructed,
                               "surface already has a buffer attached");
        return;
    }

    auto *wlOutput = WaylandOutput::fromResource(output);
    if (!wlOutput) {
        qCWarning(gLcAuroraCompositorExtSessionLockV1, "Resource wl_output@%d doesn't exist",
                  wl_resource_get_id(output));
        return;
    }

    // Only one lock surface per output is allowed
    if (managerPrivate->outputs().contains(wlOutput)) {
        qCWarning(gLcAuroraCompositorExtSessionLockV1, "Output wl_output@%d already has a lock surface on it",
                  wl_resource_get_id(output));
        wl_resource_post_error(resource->handle, error_duplicate_output,
                               "wl_output@%d already has a lock surface on it",
                               wl_resource_get_id(output));
        return;
    }

    // Set role
    if (!wlSurface->setRole(WaylandExtSessionLockSurfaceV1::role(), resource->handle, error_role))
        return;

    WaylandResource lockSurfaceResource(wl_resource_create(resource->client(),
                                                           &ext_session_lock_surface_v1_interface,
                                                           wl_resource_get_version(resource->handle), id));

    auto *lockSurface = new WaylandExtSessionLockSurfaceV1(manager, wlSurface, wlOutput, lockSurfaceResource);
    WaylandExtSessionLockManagerV1Private::get(manager)->registerLockSurface(lockSurface);
    emit manager->lockSurfaceCreated(lockSurface);
}

void WaylandExtSessionLockV1::ext_session_lock_v1_unlock_and_destroy(Resource *resource)
{
    auto *manager = qobject_cast<WaylandExtSessionLockManagerV1 *>(extensionContainer());
    Q_ASSERT(manager);

    auto *managerPrivate = WaylandExtSessionLockManagerV1Private::get(manager);
    Q_ASSERT(manager);

    if (!managerPrivate->isLocked()) {
        wl_resource_post_error(resource->handle, error_invalid_unlock,
                               "unlock requested but locked event was never sent");
        return;
    }

    // Now we can unlock the session
    managerPrivate->setLocked(false);
    managerPrivate->setClientConnected(false);

    // Destroy the resource
    wl_resource_destroy(resource->handle);
}

/*
 * WaylandExtSessionLockSurfaceV1
 */

WaylandSurfaceRole WaylandExtSessionLockSurfaceV1Private::s_role("ext_session_lock_surface_v1");

WaylandExtSessionLockSurfaceV1::WaylandExtSessionLockSurfaceV1(WaylandExtSessionLockManagerV1 *manager,
                                                               WaylandSurface *surface,
                                                               WaylandOutput *output,
                                                               const WaylandResource &resource)
    : WaylandShellSurfaceTemplate<WaylandExtSessionLockSurfaceV1>(manager)
{
    Q_D(WaylandExtSessionLockSurfaceV1);

    d->manager = manager;
    d->surface = surface;
    d->output = output;

    d->init(resource.resource());
    setExtensionContainer(surface);
    WaylandCompositorExtension::initialize();

    emit shellChanged();

    // Check if the surface is valid
    connect(surface, SIGNAL(damaged()), this, SLOT(handleSurfaceDamaged()));

    // Lock surfaces need to be as big as the whole output, in order to
    // completely occlude the other surfaces
    d->sendConfigure(d->output->geometry().size());
    connect(d->output, &WaylandOutput::geometryChanged, this, [d] {
        d->sendConfigure(d->output->geometry().size());
    });
}

WaylandExtSessionLockSurfaceV1::~WaylandExtSessionLockSurfaceV1()
{
    Q_D(WaylandExtSessionLockSurfaceV1);

    disconnect(d->output, nullptr, this, nullptr);
}

WaylandExtSessionLockManagerV1 *WaylandExtSessionLockSurfaceV1::shell() const
{
    Q_D(const WaylandExtSessionLockSurfaceV1);
    return d->manager;
}

WaylandSurface *WaylandExtSessionLockSurfaceV1::surface() const
{
    Q_D(const WaylandExtSessionLockSurfaceV1);
    return d->surface;
}

WaylandOutput *WaylandExtSessionLockSurfaceV1::output() const
{
    Q_D(const WaylandExtSessionLockSurfaceV1);
    return d->output;
}

const wl_interface *WaylandExtSessionLockSurfaceV1::interface()
{
    return WaylandExtSessionLockSurfaceV1Private::interface();
}

QByteArray WaylandExtSessionLockSurfaceV1::interfaceName()
{
    return WaylandExtSessionLockSurfaceV1Private::interfaceName();
}

WaylandSurfaceRole *WaylandExtSessionLockSurfaceV1::role()
{
    return &WaylandExtSessionLockSurfaceV1Private::s_role;
}

#if LIRI_FEATURE_aurora_compositor_quick
WaylandQuickShellIntegration *WaylandExtSessionLockSurfaceV1::createIntegration(WaylandQuickShellSurfaceItem *item)
{
    return new Internal::WaylandExtSessionLockV1Integration(item);
}
#endif

void WaylandExtSessionLockSurfaceV1::initialize()
{
    WaylandShellSurfaceTemplate::initialize();
}

/*
 * WaylandExtSessionLockSurfaceV1Private
 */

WaylandExtSessionLockSurfaceV1Private::WaylandExtSessionLockSurfaceV1Private()
{
}

quint32 WaylandExtSessionLockSurfaceV1Private::sendConfigure(const QSize &size)
{
    auto serial = surface->compositor()->nextSerial();
    pendingConfigures.append(WaylandExtSessionLockSurfaceV1Private::ConfigureEvent(size, serial));
    send_configure(serial, size.width(), size.height());
    return serial;
}

void WaylandExtSessionLockSurfaceV1Private::handleSurfaceDamaged()
{
    if (!surface->hasContent()) {
        wl_resource_post_error(resource()->handle, error_null_buffer,
                               "surface committed with a null buffer");
        return;
    }

    if (!configured) {
        wl_resource_post_error(resource()->handle, error_commit_before_first_ack,
                               "surface committed before first ack_configure request");
        return;
    }

    QSize currentSize = surface->destinationSize();
    QSize configureSize = lastAckedConfigure.size;
    if (currentSize.width() < 0 || currentSize.height() < 0 ||
            currentSize.width() != configureSize.width() ||
            currentSize.height() != configureSize.height()) {
        wl_resource_post_error(resource()->handle, error_dimensions_mismatch,
                               "failed to match ack'd surface size");
        return;
    }
}

void WaylandExtSessionLockSurfaceV1Private::ext_session_lock_surface_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(WaylandExtSessionLockSurfaceV1);
    WaylandExtSessionLockManagerV1Private::get(manager)->unregisterLockSurface(q);
    delete q;
}

void WaylandExtSessionLockSurfaceV1Private::ext_session_lock_surface_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void WaylandExtSessionLockSurfaceV1Private::ext_session_lock_surface_v1_ack_configure(Resource *resource, uint32_t serial)
{
    ConfigureEvent config;
    bool found = false;

    Q_FOREVER {
        if (pendingConfigures.isEmpty()) {
            qCWarning(gLcAuroraCompositorExtSessionLockV1, "Received an unexpected ack_configure!");
            return;
        }

        config = pendingConfigures.takeFirst();
        if (config.serial == serial) {
            found = true;
            break;
        }
    }

    if (!found) {
        wl_resource_post_error(resource->handle, error_invalid_serial,
                               "wrong configure serial: %u", serial);
        return;
    }

    lastAckedConfigure = config;

    if (!configured)
        configured = true;
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandextsessionlockv1.cpp"
