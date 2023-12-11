// SPDX-FileCopyrightText: 2022-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <LiriAuroraCompositor/WaylandResource>
#include <LiriAuroraCompositor/WaylandShell>
#include <LiriAuroraCompositor/WaylandShellSurface>
#include <LiriAuroraCompositor/WaylandSurface>

namespace Aurora {

namespace Compositor {

class WaylandExtSessionLockManagerV1Private;
class WaylandExtSessionLockSurfaceV1;
class WaylandExtSessionLockSurfaceV1Private;

class LIRIAURORACOMPOSITOR_EXPORT WaylandExtSessionLockManagerV1
        : public WaylandShellTemplate<WaylandExtSessionLockManagerV1>
{
    Q_OBJECT
    Q_PROPERTY(bool locked READ isLocked NOTIFY lockedChanged)
    Q_PROPERTY(bool hasClientConnected READ hasClientConnected NOTIFY hasClientConnectedChanged)
    Q_DECLARE_PRIVATE(WaylandExtSessionLockManagerV1)
public:
    WaylandExtSessionLockManagerV1();
    WaylandExtSessionLockManagerV1(WaylandCompositor *compositor);
    ~WaylandExtSessionLockManagerV1();

    bool isLocked() const;
    bool hasClientConnected() const;

    void initialize() override;

    static const wl_interface *interface();
    static QByteArray interfaceName();

signals:
    void lockedChanged();
    void hasClientConnectedChanged();
    void lockSurfaceCreated(Aurora::Compositor::WaylandExtSessionLockSurfaceV1 *lockSurface);

private:
    QScopedPointer<WaylandExtSessionLockManagerV1Private> const d_ptr;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandExtSessionLockSurfaceV1
        : public WaylandShellSurfaceTemplate<WaylandExtSessionLockSurfaceV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandExtSessionLockSurfaceV1)
    Q_PROPERTY(Aurora::Compositor::WaylandExtSessionLockManagerV1 *shell READ shell NOTIFY shellChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandSurface *surface READ surface CONSTANT)
    Q_PROPERTY(Aurora::Compositor::WaylandOutput *output READ output CONSTANT)
public:
    WaylandExtSessionLockSurfaceV1(WaylandExtSessionLockManagerV1 *manager,
                                   WaylandSurface *surface,
                                   WaylandOutput *output,
                                   const WaylandResource &resource);
    ~WaylandExtSessionLockSurfaceV1();

    WaylandExtSessionLockManagerV1 *shell() const override;
    WaylandSurface *surface() const;
    WaylandOutput *output() const;

    static const wl_interface *interface();
    static QByteArray interfaceName();
    static WaylandSurfaceRole *role();

#if LIRI_FEATURE_aurora_compositor_quick
    WaylandQuickShellIntegration *createIntegration(WaylandQuickShellSurfaceItem *item) override;
#endif

private:
    QScopedPointer<WaylandExtSessionLockSurfaceV1Private> const d_ptr;

    Q_PRIVATE_SLOT(d_func(), void handleSurfaceDamaged());

    void initialize() override;
};

} // namespace Compositor

} // namespace Aurora

