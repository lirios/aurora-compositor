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
    void wlShellSurfaceRequested(WaylandSurface *surface, const WaylandResource &resource);
    void wlShellSurfaceCreated(WaylandWlShellSurface *shellSurface);
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

    Q_INVOKABLE void initialize(WaylandWlShell *shell, WaylandSurface *surface, const WaylandResource &resource);

    QString title() const;
    QString className() const;

    WaylandSurface *surface() const;
    WaylandWlShell *shell() const;

    Qt::WindowType windowType() const override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
    static WaylandSurfaceRole *role();

    static WaylandWlShellSurface *fromResource(wl_resource *res);

    Q_INVOKABLE QSize sizeForResize(const QSizeF &size, const QPointF &delta, ResizeEdge edges);
    Q_INVOKABLE void sendConfigure(const QSize &size, ResizeEdge edges);
    Q_INVOKABLE void sendPopupDone();

#if LIRI_FEATURE_aurora_compositor_quick
    WaylandQuickShellIntegration *createIntegration(WaylandQuickShellSurfaceItem *item) override;
#endif

public Q_SLOTS:
    void ping();

Q_SIGNALS:
    void surfaceChanged();
    void shellChanged();
    void titleChanged();
    void classNameChanged();
    void pong();
    void startMove(WaylandSeat *seat);
    void startResize(WaylandSeat *seat, ResizeEdge edges);

    void setDefaultToplevel();
    void setTransient(WaylandSurface *parentSurface, const QPoint &relativeToParent, bool inactive);
    void setFullScreen(FullScreenMethod method, uint framerate, WaylandOutput *output);
    void setPopup(WaylandSeat *seat, WaylandSurface *parentSurface, const QPoint &relativeToParent);
    void setMaximized(WaylandOutput *output);

private:
    void initialize() override;
};

} // namespace Compositor

} // namespace Aurora

