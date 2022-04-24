/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef AURORA_COMPOSITOR_WAYLANDXDGDECORATIONV1_P_H
#define AURORA_COMPOSITOR_WAYLANDXDGDECORATIONV1_P_H

#include "aurorawaylandxdgdecorationv1.h"

#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-xdg-decoration-unstable-v1.h>

#include <LiriAuroraCompositor/WaylandXdgToplevel>

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

class WaylandXdgToplevel;

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgDecorationManagerV1Private
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::zxdg_decoration_manager_v1
{
    Q_DECLARE_PUBLIC(WaylandXdgDecorationManagerV1)
public:
    using DecorationMode = WaylandXdgToplevel::DecorationMode;
    explicit WaylandXdgDecorationManagerV1Private(WaylandXdgDecorationManagerV1 *self);

protected:
    void zxdg_decoration_manager_v1_get_toplevel_decoration(Resource *resource, uint id, ::wl_resource *toplevelResource) override;

private:
    DecorationMode m_preferredMode = DecorationMode::ClientSideDecoration;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgToplevelDecorationV1
        : public PrivateServer::zxdg_toplevel_decoration_v1
{
public:
    using DecorationMode = WaylandXdgToplevel::DecorationMode;
    explicit WaylandXdgToplevelDecorationV1(WaylandXdgToplevel *toplevel,
                                             WaylandXdgDecorationManagerV1 *manager,
                                             wl_client *client, int id);
    ~WaylandXdgToplevelDecorationV1() override;

    DecorationMode configuredMode() const { return DecorationMode(m_configuredMode); }
    void sendConfigure(DecorationMode mode);

protected:
    void zxdg_toplevel_decoration_v1_destroy_resource(Resource *resource) override;
    void zxdg_toplevel_decoration_v1_destroy(Resource *resource) override;
    void zxdg_toplevel_decoration_v1_set_mode(Resource *resource, uint32_t mode) override;
    void zxdg_toplevel_decoration_v1_unset_mode(Resource *resource) override;

private:
    void handleClientPreferredModeChanged();

    WaylandXdgToplevel *m_toplevel = nullptr;
    WaylandXdgDecorationManagerV1 *m_manager = nullptr;
    uint m_configuredMode = 0;
    uint m_clientPreferredMode = 0;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDXDGDECORATIONV1_P_H
