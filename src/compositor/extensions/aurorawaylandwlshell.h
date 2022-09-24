/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef AURORA_COMPOSITOR_WAYLANDWLSHELL_H
#define AURORA_COMPOSITOR_WAYLANDWLSHELL_H

#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/WaylandResource>
#include <LiriAuroraCompositor/WaylandShell>
#include <LiriAuroraCompositor/WaylandShellSurface>
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>

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
    ~WaylandWlShell();

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

private:
    QScopedPointer<WaylandWlShellPrivate> const d_ptr;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlShellSurface : public WaylandShellSurfaceTemplate<WaylandWlShellSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlShellSurface)
    AURORA_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandWlShellSurface)
    Q_PROPERTY(Aurora::Compositor::WaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandWlShell *shell READ shell NOTIFY shellChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString className READ className NOTIFY classNameChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    Q_MOC_INCLUDE("aurorawaylandsurface.h")
#endif
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
    QScopedPointer<WaylandWlShellSurfacePrivate> const d_ptr;

    void initialize() override;
};

} // namespace Compositor

} // namespace Aurora

#endif  /*QWAYLANDWLSHELL_H*/
