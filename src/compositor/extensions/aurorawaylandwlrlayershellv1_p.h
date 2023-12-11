// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QLoggingCategory>
#include <QSize>

#include <LiriAuroraCompositor/WaylandWlrLayerShellV1>
#include <LiriAuroraCompositor/private/aurorawaylandshell_p.h>
#include <LiriAuroraCompositor/private/aurora-server-wlr-layer-shell-unstable-v1.h>

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

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrLayerShellV1Private
        : public WaylandShellPrivate
        , public PrivateServer::zwlr_layer_shell_v1
{
    Q_DECLARE_PUBLIC(WaylandWlrLayerShellV1)
public:
    explicit WaylandWlrLayerShellV1Private(WaylandWlrLayerShellV1 *self);

    void unregisterLayerSurface(WaylandWlrLayerSurfaceV1 *layerSurface);
    void closeAllLayerSurfaces();

    static WaylandWlrLayerShellV1Private *get(WaylandWlrLayerShellV1 *shell) { return shell->d_func(); }

protected:
    QList<WaylandWlrLayerSurfaceV1 *> m_layerSurfaces;

    void zwlr_layer_shell_v1_get_layer_surface(Resource *resource, uint32_t id,
                                               struct ::wl_resource *surfaceRes,
                                               struct ::wl_resource *outputRes,
                                               uint32_t layer,
                                               const QString &nameSpace) override;

};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrLayerSurfaceV1Private
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::zwlr_layer_surface_v1
{
    Q_DECLARE_PUBLIC(WaylandWlrLayerSurfaceV1)
public:
    struct State {
        State() = default;

        WaylandWlrLayerShellV1::Layer layer = WaylandWlrLayerShellV1::BackgroundLayer;
        QSize desiredSize = QSize(0, 0);
        WaylandWlrLayerSurfaceV1::Anchors anchors;
        int exclusiveZone = 0;
        QMargins margins;
        WaylandWlrLayerSurfaceV1::KeyboardInteractivity keyboardInteractivity = WaylandWlrLayerSurfaceV1::NoKeyboardInteractivity;
    };

    struct ConfigureEvent {
        ConfigureEvent() = default;
        ConfigureEvent(const QSize &incomingSize, quint32 incomingSerial)
            : serial(incomingSerial)
            , size(incomingSize)
        { }

        quint32 serial = 0;
        QSize size = QSize(0, 0);
    };

    explicit WaylandWlrLayerSurfaceV1Private(WaylandWlrLayerSurfaceV1 *self);
    ~WaylandWlrLayerSurfaceV1Private();

    ConfigureEvent lastSentConfigure() const;

    void unmap();

    static WaylandWlrLayerSurfaceV1Private *get(WaylandWlrLayerSurfaceV1 *surface) { return surface->d_func(); }

    WaylandWlrLayerShellV1 *shell = nullptr;
    WaylandSurface *surface = nullptr;
    WaylandOutput *output = nullptr;
    QString nameSpace;

    bool added = false;
    bool configured = false;
    bool mapped = false;
    bool closed = false;

    State clientPending, serverPending, current;

    QVector<ConfigureEvent> pendingConfigures;
    ConfigureEvent lastAckedConfigure;

    static WaylandSurfaceRole s_role;

protected:
    void zwlr_layer_surface_v1_set_size(Resource *resource, uint32_t width,
                                        uint32_t height) override;
    void zwlr_layer_surface_v1_set_anchor(Resource *resource,
                                          uint32_t anchor) override;
    void zwlr_layer_surface_v1_set_exclusive_zone(Resource *resource, int32_t zone) override;
    void zwlr_layer_surface_v1_set_margin(Resource *resource, int32_t top, int32_t right, int32_t bottom, int32_t left) override;
    void zwlr_layer_surface_v1_set_keyboard_interactivity(Resource *resource, uint32_t keyboard_interactivity) override;
    void zwlr_layer_surface_v1_get_popup(Resource *resource, struct ::wl_resource *popup) override;
    void zwlr_layer_surface_v1_ack_configure(Resource *resource, uint32_t serial) override;
    void zwlr_layer_surface_v1_set_layer(Resource *resource, uint32_t layer) override;
};

} // namespace Compositor

} // namespace Aurora

