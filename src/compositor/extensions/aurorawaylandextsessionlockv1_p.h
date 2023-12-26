// SPDX-FileCopyrightText: 2022-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtCore/QPointer>

#include <LiriAuroraCompositor/WaylandExtSessionLockManagerV1>
#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandshell_p.h>
#include <LiriAuroraCompositor/private/aurora-server-ext-session-lock-v1.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Aurora API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandExtSessionLockV1
        : public WaylandShellTemplate<WaylandExtSessionLockV1>
        , public PrivateServer::ext_session_lock_v1
{
public:
    WaylandExtSessionLockV1(WaylandExtSessionLockManagerV1 *manager,
                            const WaylandResource &resource);
    ~WaylandExtSessionLockV1();

protected:
    void ext_session_lock_v1_destroy_resource(Resource *resource) override;
    void ext_session_lock_v1_destroy(Resource *resource) override;
    void ext_session_lock_v1_get_lock_surface(Resource *resource, uint32_t id,
                                              struct ::wl_resource *surface,
                                              struct ::wl_resource *output) override;
    void ext_session_lock_v1_unlock_and_destroy(Resource *resource) override;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandExtSessionLockManagerV1Private
        : public WaylandShellPrivate
        , public PrivateServer::ext_session_lock_manager_v1
{
    Q_DECLARE_PUBLIC(WaylandExtSessionLockManagerV1)
public:
    WaylandExtSessionLockManagerV1Private();

    bool isLocked() const;
    void setLocked(bool value);

    void setClientConnected(bool value);

    void setClient(WaylandClient *client);

    QList<WaylandOutput *> outputs() const;

    void registerLockSurface(WaylandExtSessionLockSurfaceV1 *lockSurface);
    void unregisterLockSurface(WaylandExtSessionLockSurfaceV1 *lockSurface);

    static WaylandExtSessionLockManagerV1Private *get(WaylandExtSessionLockManagerV1 *manager) { return manager ? manager->d_func() : nullptr; }

    QPointer<WaylandExtSessionLockV1> sessionLock;

protected:
    void ext_session_lock_manager_v1_bind_resource(Resource *resource) override;
    void ext_session_lock_manager_v1_destroy_resource(Resource *resource) override;
    void ext_session_lock_manager_v1_destroy(Resource *resource) override;
    void ext_session_lock_manager_v1_lock(Resource *resource, uint32_t id) override;

private:
    WaylandClient *m_client = nullptr;
    bool m_locked = false;
    bool m_clientConnected = false;
    QMultiMap<struct wl_client *, WaylandExtSessionLockSurfaceV1 *> m_lockSurfaces;
    QList<WaylandOutput *> m_outputs;
    QMap<WaylandSeat *, WaylandSurface *> m_oldFocus;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandExtSessionLockSurfaceV1Private
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::ext_session_lock_surface_v1
{
    Q_DECLARE_PUBLIC(WaylandExtSessionLockSurfaceV1)
public:
    struct ConfigureEvent {
        ConfigureEvent() = default;
        ConfigureEvent(const QSize &incomingSize, quint32 incomingSerial)
            : serial(incomingSerial)
            , size(incomingSize)
        { }

        quint32 serial = 0;
        QSize size = QSize(0, 0);
    };

    WaylandExtSessionLockSurfaceV1Private();

    quint32 sendConfigure(const QSize &size);

    void handleSurfaceDamaged();

    static WaylandExtSessionLockSurfaceV1Private *get(WaylandExtSessionLockSurfaceV1 *lockSurface) { return lockSurface ? lockSurface->d_func() : nullptr; }

    WaylandExtSessionLockManagerV1 *manager = nullptr;
    WaylandSurface *surface = nullptr;
    WaylandOutput *output = nullptr;

    bool configured = false;
    QVector<ConfigureEvent> pendingConfigures;
    ConfigureEvent lastAckedConfigure;

    static WaylandSurfaceRole s_role;

protected:
    void ext_session_lock_surface_v1_destroy_resource(Resource *resource) override;
    void ext_session_lock_surface_v1_destroy(Resource *resource) override;
    void ext_session_lock_surface_v1_ack_configure(Resource *resource, uint32_t serial) override;
};

} // namespace Compositor

} // namespace Aurora

