/****************************************************************************
**
** Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef AURORA_COMPOSITOR_WAYLANDIDLEINHIBITV1_P_H
#define AURORA_COMPOSITOR_WAYLANDIDLEINHIBITV1_P_H

#include <LiriAuroraCompositor/WaylandSurface>
#include <LiriAuroraCompositor/WaylandIdleInhibitManagerV1>
#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-idle-inhibit-unstable-v1.h>

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

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandIdleInhibitManagerV1Private
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::zwp_idle_inhibit_manager_v1
{
    Q_DECLARE_PUBLIC(WaylandIdleInhibitManagerV1)
public:
    explicit WaylandIdleInhibitManagerV1Private() = default;

    class Q_WAYLANDCOMPOSITOR_EXPORT Inhibitor
            : public PrivateServer::zwp_idle_inhibitor_v1
    {
    public:
        explicit Inhibitor(WaylandSurface *surface, wl_client *client, quint32 id, quint32 version);

    protected:
        void zwp_idle_inhibitor_v1_destroy_resource(Resource *resource) override;
        void zwp_idle_inhibitor_v1_destroy(Resource *resource) override;

    private:
        QPointer<WaylandSurface> m_surface;
    };

    static WaylandIdleInhibitManagerV1Private *get(WaylandIdleInhibitManagerV1 *manager) { return manager ? manager->d_func() : nullptr; }

protected:
    void zwp_idle_inhibit_manager_v1_create_inhibitor(Resource *resource, uint32_t id, wl_resource *surfaceResource) override;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDIDLEINHIBITV1_P_H
