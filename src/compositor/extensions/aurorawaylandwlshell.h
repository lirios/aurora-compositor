// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/WaylandResource>
#include <LiriAuroraCompositor/WaylandShell>
#include <LiriAuroraCompositor/WaylandShellSurface>
#if LIRI_FEATURE_aurora_compositor_quick
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>
#endif

#include <QtCore/QSize>

namespace Aurora {

namespace Compositor {

class WaylandWlShellPrivate;
class WaylandWlShellSurfacePrivate;
class WaylandSurface;
class WaylandClient;
class WaylandSeat;
class WaylandOutput;
class WaylandSurfaceRole;
class WaylandWlShellSurface;

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlShell : public WaylandShellTemplate<WaylandWlShell>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlShell)
public:
    WaylandWlShell();
    WaylandWlShell(WaylandCompositor *compositor);

    void initialize() override;
    QList<WaylandWlShellSurface *> shellSurfaces() const;
    QList<WaylandWlShellSurface *> shellSurfacesForClient(WaylandClient* client) const;
    QList<WaylandWlShellSurface *> mappedPopups() const;
    WaylandClient *popupClient() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

public Q_SLOTS:
    void closeAllPopups();

Q_SIGNALS:
    void wlShellSurfaceRequested(Aurora::Compositor::WaylandSurface *surface, const Aurora::Compositor::WaylandResource &resource);
    void wlShellSurfaceCreated(Aurora::Compositor::WaylandWlShellSurface *shellSurface);

};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlShellSurface : public WaylandShellSurfaceTemplate<WaylandWlShellSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlShellSurface)
#if LIRI_FEATURE_aurora_compositor_quick
    AURORA_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandWlShellSurface)
#endif
    Q_PROPERTY(Aurora::Compositor::WaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandWlShell *shell READ shell NOTIFY shellChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString className READ className NOTIFY classNameChanged)
    Q_MOC_INCLUDE("aurorawaylandsurface.h")

public:
    enum FullScreenMethod {
        DefaultFullScreen,
        ScaleFullScreen,
        DriverFullScreen,
        FillFullScreen
    };
    Q_ENUM(FullScreenMethod);

    enum ResizeEdge {
        NoneEdge        =  0,
        TopEdge         =  1,
        BottomEdge      =  2,
        LeftEdge        =  4,
        TopLeftEdge     =  5,
        BottomLeftEdge  =  6,
        RightEdge       =  8,
        TopRightEdge    =  9,
        BottomRightEdge = 10
    };
    Q_ENUM(ResizeEdge);

    WaylandWlShellSurface();
    WaylandWlShellSurface(WaylandWlShell *shell, WaylandSurface *surface, const WaylandResource &resource);
    ~WaylandWlShellSurface() override;

    Q_INVOKABLE void initialize(Aurora::Compositor::WaylandWlShell *shell, Aurora::Compositor::WaylandSurface *surface, const Aurora::Compositor::WaylandResource &resource);

    QString title() const;
    QString className() const;

    WaylandSurface *surface() const;
    WaylandWlShell *shell() const override;

    Qt::WindowType windowType() const override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
    static WaylandSurfaceRole *role();

    static WaylandWlShellSurface *fromResource(wl_resource *res);

    Q_INVOKABLE QSize sizeForResize(const QSizeF &size, const QPointF &delta, Aurora::Compositor::WaylandWlShellSurface::ResizeEdge edges);
    Q_INVOKABLE void sendConfigure(const QSize &size, Aurora::Compositor::WaylandWlShellSurface::ResizeEdge edges);
    Q_INVOKABLE void sendPopupDone();

#if LIRI_FEATURE_aurora_compositor_quick
    WaylandQuickShellIntegration *createIntegration(WaylandQuickShellSurfaceItem *item) override;
#endif

public Q_SLOTS:
    void ping();

Q_SIGNALS:
    void surfaceChanged();
    void titleChanged();
    void classNameChanged();
    void pong();
    void startMove(Aurora::Compositor::WaylandSeat *seat);
    void startResize(Aurora::Compositor::WaylandSeat *seat, Aurora::Compositor::WaylandWlShellSurface::ResizeEdge edges);

    void setDefaultToplevel();
    void setTransient(Aurora::Compositor::WaylandSurface *parentSurface, const QPoint &relativeToParent, bool inactive);
    void setFullScreen(Aurora::Compositor::WaylandWlShellSurface::FullScreenMethod method, uint framerate, Aurora::Compositor::WaylandOutput *output);
    void setPopup(Aurora::Compositor::WaylandSeat *seat, Aurora::Compositor::WaylandSurface *parentSurface, const QPoint &relativeToParent);
    void setMaximized(Aurora::Compositor::WaylandOutput *output);

private:
    void initialize() override;
};

} // namespace Compositor

} // namespace Aurora

