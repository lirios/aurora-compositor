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

#ifndef AURORA_COMPOSITOR_WAYLANDIDLEINHIBITV1_H
#define AURORA_COMPOSITOR_WAYLANDIDLEINHIBITV1_H

#include <LiriAuroraCompositor/WaylandCompositorExtension>

namespace Aurora {

namespace Compositor {

class WaylandIdleInhibitManagerV1Private;

class LIRIAURORACOMPOSITOR_EXPORT WaylandIdleInhibitManagerV1 : public WaylandCompositorExtensionTemplate<WaylandIdleInhibitManagerV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandIdleInhibitManagerV1)
public:
    WaylandIdleInhibitManagerV1();
    explicit WaylandIdleInhibitManagerV1(WaylandCompositor *compositor);
    ~WaylandIdleInhibitManagerV1();

    void initialize() override;

    static const struct wl_interface *interface();
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDIDLEINHIBITV1_H
