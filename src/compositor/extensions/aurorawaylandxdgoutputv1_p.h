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

#ifndef AURORA_COMPOSITOR_WAYLANDXDGOUTPUTV1_P_H
#define AURORA_COMPOSITOR_WAYLANDXDGOUTPUTV1_P_H

#include <QtCore/QHash>

#include <LiriAuroraCompositor/WaylandOutput>
#include <LiriAuroraCompositor/WaylandXdgOutputV1>
#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-xdg-output-unstable-v1.h>

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

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgOutputManagerV1Private
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::zxdg_output_manager_v1
{
    Q_DECLARE_PUBLIC(WaylandXdgOutputManagerV1)
public:
    explicit WaylandXdgOutputManagerV1Private() = default;

    void registerXdgOutput(WaylandOutput *output, WaylandXdgOutputV1 *xdgOutput);
    void unregisterXdgOutput(WaylandOutput *output);

    static WaylandXdgOutputManagerV1Private *get(WaylandXdgOutputManagerV1 *manager) { return manager ? manager->d_func() : nullptr; }

protected:
    void zxdg_output_manager_v1_get_xdg_output(Resource *resource, uint32_t id,
                                               wl_resource *outputResource) override;

private:
    QHash<WaylandOutput *, WaylandXdgOutputV1 *> xdgOutputs;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgOutputV1Private
        : public QObjectPrivate
        , public PrivateServer::zxdg_output_v1
{
    Q_DECLARE_PUBLIC(WaylandXdgOutputV1)
public:
    explicit WaylandXdgOutputV1Private() = default;

    void sendLogicalPosition(const QPoint &position);
    void sendLogicalSize(const QSize &size);
    void sendDone();

    void setManager(WaylandXdgOutputManagerV1 *manager);
    void setOutput(WaylandOutput *output);

    static WaylandXdgOutputV1Private *get(WaylandXdgOutputV1 *xdgOutput) { return xdgOutput ? xdgOutput->d_func() : nullptr; }

    bool initialized = false;
    WaylandOutput *output = nullptr;
    WaylandXdgOutputManagerV1 *manager = nullptr;
    QPoint logicalPos;
    QSize logicalSize;
    QString name;
    QString description;
    bool needToSendDone = false;

protected:
    void zxdg_output_v1_bind_resource(Resource *resource) override;
    void zxdg_output_v1_destroy(Resource *resource) override;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDXDGOUTPUTV1_P_H
