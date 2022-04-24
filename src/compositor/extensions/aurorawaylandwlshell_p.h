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

#ifndef AURORA_COMPOSITOR_WAYLANDWLSHELL_P_H
#define AURORA_COMPOSITOR_WAYLANDWLSHELL_P_H

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/aurorawaylandsurface.h>
#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandshell_p.h>
#include <LiriAuroraCompositor/WaylandWlShellSurface>
#include <LiriAuroraCompositor/WaylandSeat>

#include <wayland-server-core.h>
#include <QHash>
#include <QPoint>
#include <QSet>

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>

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

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlShellPrivate
                                        : public WaylandShellPrivate
                                        , public PrivateServer::wl_shell
{
    Q_DECLARE_PUBLIC(WaylandWlShell)
public:
    WaylandWlShellPrivate(WaylandWlShell *self);

    void unregisterShellSurface(WaylandWlShellSurface *shellSurface);

    static WaylandWlShellPrivate *get(WaylandWlShell *shell) { return shell->d_func(); }

protected:
    void shell_get_shell_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface) override;

    QList<WaylandWlShellSurface *> m_shellSurfaces;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlShellSurfacePrivate
                                        : public WaylandCompositorExtensionPrivate
                                        , public PrivateServer::wl_shell_surface
{
    Q_DECLARE_PUBLIC(WaylandWlShellSurface)
public:
    WaylandWlShellSurfacePrivate(WaylandWlShellSurface *self);
    ~WaylandWlShellSurfacePrivate() override;

    static WaylandWlShellSurfacePrivate *get(WaylandWlShellSurface *surface) { return surface->d_func(); }

    void ping(uint32_t serial);

    void setWindowType(Qt::WindowType windowType);

private:
    WaylandWlShell *m_shell = nullptr;
    QPointer<WaylandSurface> m_surface;

    QSet<uint32_t> m_pings;

    QString m_title;
    QString m_className;
    Qt::WindowType m_windowType = Qt::WindowType::Window;

    void shell_surface_destroy_resource(Resource *resource) override;

    void shell_surface_move(Resource *resource,
                            struct wl_resource *input_device_super,
                            uint32_t time) override;
    void shell_surface_resize(Resource *resource,
                              struct wl_resource *input_device,
                              uint32_t time,
                              uint32_t edges) override;
    void shell_surface_set_toplevel(Resource *resource) override;
    void shell_surface_set_transient(Resource *resource,
                                     struct wl_resource *parent_surface_resource,
                                     int x,
                                     int y,
                                     uint32_t flags) override;
    void shell_surface_set_fullscreen(Resource *resource,
                                      uint32_t method,
                                      uint32_t framerate,
                                      struct wl_resource *output_resource) override;
    void shell_surface_set_popup(Resource *resource,
                                 struct wl_resource *input_device,
                                 uint32_t time,
                                 struct wl_resource *parent,
                                 int32_t x,
                                 int32_t y,
                                 uint32_t flags) override;
    void shell_surface_set_maximized(Resource *resource,
                                     struct wl_resource *output_resource) override;
    void shell_surface_pong(Resource *resource,
                            uint32_t serial) override;
    void shell_surface_set_title(Resource *resource,
                                 const QString &title) override;
    void shell_surface_set_class(Resource *resource,
                                 const QString &class_) override;

    static WaylandSurfaceRole s_role;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDWLSHELL_P_H
