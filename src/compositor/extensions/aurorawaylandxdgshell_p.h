// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandshell_p.h>
#include <LiriAuroraCompositor/private/aurora-server-xdg-shell.h>

#include <LiriAuroraCompositor/WaylandXdgShell>

#include <LiriAuroraCompositor/private/aurorawaylandxdgdecorationv1_p.h>

#include <QtCore/QSet>

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

struct LIRIAURORACOMPOSITOR_EXPORT WaylandXdgPositionerData {
    QSize size;
    QRect anchorRect;
    Qt::Edges anchorEdges = {};
    Qt::Edges gravityEdges = {};
    uint constraintAdjustments = XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_NONE;
    QPoint offset;
    WaylandXdgPositionerData();
    bool isComplete() const;
    QPoint anchorPoint() const;
    QPoint unconstrainedPosition() const;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgShellPrivate
        : public WaylandShellPrivate
        , public PrivateServer::xdg_wm_base
{
    Q_DECLARE_PUBLIC(WaylandXdgShell)
public:
    WaylandXdgShellPrivate(WaylandXdgShell *self);
    void ping(Resource *resource, uint32_t serial);
    void registerXdgSurface(WaylandXdgSurface *xdgSurface);
    void unregisterXdgSurface(WaylandXdgSurface *xdgSurface);
    static WaylandXdgShellPrivate *get(WaylandXdgShell *xdgShell) { return xdgShell->d_func(); }

    QSet<uint32_t> m_pings;
    QMultiMap<struct wl_client *, WaylandXdgSurface *> m_xdgSurfaces;

    WaylandXdgSurface *xdgSurfaceFromSurface(WaylandSurface *surface);

protected:
    void xdg_wm_base_destroy(Resource *resource) override;
    void xdg_wm_base_create_positioner(Resource *resource, uint32_t id) override;
    void xdg_wm_base_get_xdg_surface(Resource *resource, uint32_t id,
                                     struct ::wl_resource *surface) override;
    void xdg_wm_base_pong(Resource *resource, uint32_t serial) override;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgSurfacePrivate
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::xdg_surface
{
    Q_DECLARE_PUBLIC(WaylandXdgSurface)
public:
    WaylandXdgSurfacePrivate(WaylandXdgSurface *self);
    void setWindowType(Qt::WindowType windowType);
    void handleFocusLost();
    void handleFocusReceived();
    static WaylandXdgSurfacePrivate *get(WaylandXdgSurface *xdgSurface) { return xdgSurface->d_func(); }

    QRect calculateFallbackWindowGeometry() const;
    void updateFallbackWindowGeometry();

private:
    WaylandXdgShell *m_xdgShell = nullptr;
    WaylandSurface *m_surface = nullptr;

    WaylandXdgToplevel *m_toplevel = nullptr;
    WaylandXdgPopup *m_popup = nullptr;
    QRect m_windowGeometry;
    bool m_unsetWindowGeometry = true;
    QMargins m_windowMargins;
    Qt::WindowType m_windowType = Qt::WindowType::Window;

    void xdg_surface_destroy_resource(Resource *resource) override;
    void xdg_surface_destroy(Resource *resource) override;
    void xdg_surface_get_toplevel(Resource *resource, uint32_t id) override;
    void xdg_surface_get_popup(Resource *resource, uint32_t id, struct ::wl_resource *parent, struct ::wl_resource *positioner) override;
    void xdg_surface_ack_configure(Resource *resource, uint32_t serial) override;
    void xdg_surface_set_window_geometry(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgToplevelPrivate : public PrivateServer::xdg_toplevel
{
    Q_DECLARE_PUBLIC(WaylandXdgToplevel)
public:
    struct ConfigureEvent {
        ConfigureEvent() = default;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        ConfigureEvent(const QList<WaylandXdgToplevel::State>
#else
        ConfigureEvent(const QVector<WaylandXdgToplevel::State>
#endif
                       &incomingStates,
                       const QSize &incomingSize, uint incomingSerial)
        : states(incomingStates), size(incomingSize), serial(incomingSerial)
        { }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QList<WaylandXdgToplevel::State> states;
#else
        QVector<WaylandXdgToplevel::State> states;
#endif
        QSize size = {0, 0};
        uint serial = 0;
    };

    WaylandXdgToplevelPrivate(WaylandXdgToplevel *self, WaylandXdgSurface *xdgSurface, const WaylandResource& resource);
    ConfigureEvent lastSentConfigure() const { return m_pendingConfigures.empty() ? m_lastAckedConfigure : m_pendingConfigures.last(); }
    void handleAckConfigure(uint serial); //TODO: move?
    void handleFocusLost();
    void handleFocusReceived();

    static WaylandXdgToplevelPrivate *get(WaylandXdgToplevel *toplevel) { return toplevel->d_func(); }
    static Qt::Edges convertToEdges(resize_edge edge);

protected:

    void xdg_toplevel_destroy_resource(Resource *resource) override;

    void xdg_toplevel_destroy(Resource *resource) override;
    void xdg_toplevel_set_parent(Resource *resource, struct ::wl_resource *parent) override;
    void xdg_toplevel_set_title(Resource *resource, const QString &title) override;
    void xdg_toplevel_set_app_id(Resource *resource, const QString &app_id) override;
    void xdg_toplevel_show_window_menu(Resource *resource, struct ::wl_resource *seat, uint32_t serial, int32_t x, int32_t y) override;
    void xdg_toplevel_move(Resource *resource, struct ::wl_resource *seatResource, uint32_t serial) override;
    void xdg_toplevel_resize(Resource *resource, struct ::wl_resource *seat, uint32_t serial, uint32_t edges) override;
    void xdg_toplevel_set_max_size(Resource *resource, int32_t width, int32_t height) override;
    void xdg_toplevel_set_min_size(Resource *resource, int32_t width, int32_t height) override;
    void xdg_toplevel_set_maximized(Resource *resource) override;
    void xdg_toplevel_unset_maximized(Resource *resource) override;
    void xdg_toplevel_set_fullscreen(Resource *resource, struct ::wl_resource *output) override;
    void xdg_toplevel_unset_fullscreen(Resource *resource) override;
    void xdg_toplevel_set_minimized(Resource *resource) override;

public:
    WaylandXdgSurface *m_xdgSurface = nullptr;
    WaylandXdgToplevel *m_parentToplevel = nullptr;
    QList<ConfigureEvent> m_pendingConfigures;
    ConfigureEvent m_lastAckedConfigure;
    QString m_title;
    QString m_appId;
    QSize m_maxSize;
    QSize m_minSize = {0, 0};
    WaylandXdgToplevelDecorationV1 *m_decoration = nullptr;

    static WaylandSurfaceRole s_role;

private:
    WaylandXdgToplevel *q_ptr = nullptr;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgPopupPrivate : public PrivateServer::xdg_popup
{
    Q_DECLARE_PUBLIC(WaylandXdgPopup)
public:
    struct ConfigureEvent {
        QRect geometry;
        uint serial;
    };

    WaylandXdgPopupPrivate(WaylandXdgPopup *self, WaylandXdgSurface *xdgSurface, WaylandXdgSurface *parentXdgSurface,
                            WaylandXdgPositioner *positioner, const WaylandResource& resource);
    ~WaylandXdgPopupPrivate();

    void handleAckConfigure(uint serial);
    void setParentSurface(WaylandSurface *surface);

    static WaylandXdgPopupPrivate *get(WaylandXdgPopup *popup) { return popup->d_func(); }

    static WaylandSurfaceRole s_role;

private:
    uint sendConfigure(const QRect &geometry);

protected:
    void xdg_popup_destroy(Resource *resource) override;
    void xdg_popup_grab(Resource *resource, struct ::wl_resource *seat, uint32_t serial) override;

private:
    WaylandXdgPopup *q_ptr = nullptr;
    WaylandXdgSurface *m_xdgSurface = nullptr;
    WaylandXdgSurface *m_parentXdgSurface = nullptr;
    WaylandSurface *m_parentSurface = nullptr;
    WaylandXdgPositionerData m_positionerData;
    QRect m_geometry;
    QList<ConfigureEvent> m_pendingConfigures;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgPositioner : public PrivateServer::xdg_positioner
{
public:
    WaylandXdgPositioner(const WaylandResource& resource);
    static WaylandXdgPositioner *fromResource(wl_resource *resource);
    static Qt::Edges convertToEdges(anchor anchor);
    static Qt::Edges convertToEdges(gravity gravity);

protected:
    void xdg_positioner_destroy_resource(Resource *resource) override; //TODO: do something special here?

    void xdg_positioner_destroy(Resource *resource) override;
    void xdg_positioner_set_size(Resource *resource, int32_t width, int32_t height) override;
    void xdg_positioner_set_anchor_rect(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void xdg_positioner_set_anchor(Resource *resource, uint32_t anchor) override;
    void xdg_positioner_set_gravity(Resource *resource, uint32_t gravity) override;
    void xdg_positioner_set_constraint_adjustment(Resource *resource, uint32_t constraint_adjustment) override;
    void xdg_positioner_set_offset(Resource *resource, int32_t x, int32_t y) override;

public:
    WaylandXdgPositionerData m_data;
};

} // namespace Compositor

} // namespace Aurora

