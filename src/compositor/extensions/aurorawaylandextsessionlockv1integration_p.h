// SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandQuickShellSurfaceItem>
#include <LiriAuroraCompositor/private/aurorawaylandquickshellsurfaceitem_p.h>

namespace Aurora {

namespace Compositor {

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

class WaylandExtSessionLockSurfaceV1;

namespace Internal {

class WaylandExtSessionLockV1Integration : public WaylandQuickShellIntegration
{
    Q_OBJECT
public:
    WaylandExtSessionLockV1Integration(WaylandQuickShellSurfaceItem *item);

private:
    WaylandExtSessionLockSurfaceV1 *m_lockSurface = nullptr;
};

} // nmamespace Internal

} // namespace Compositor

} // namespace Aurora

