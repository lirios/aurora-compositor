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

#ifndef AURORA_COMPOSITOR_WAYLANDIVIAPPLICATION_H
#define AURORA_COMPOSITOR_WAYLANDIVIAPPLICATION_H

#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/WaylandSurface>
#include <LiriAuroraCompositor/WaylandResource>
#include <LiriAuroraCompositor/WaylandIviSurface>
#include <QtCore/QSize>

namespace Aurora {

namespace Compositor {

class WaylandIviApplicationPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandIviApplication : public WaylandCompositorExtensionTemplate<WaylandIviApplication>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandIviApplication)

public:
    WaylandIviApplication();
    WaylandIviApplication(WaylandCompositor *compositor);

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void iviSurfaceRequested(WaylandSurface *surface, uint iviId, const WaylandResource &resource);
    void iviSurfaceCreated(WaylandIviSurface *iviSurface);
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDIVIAPPLICATION_H
