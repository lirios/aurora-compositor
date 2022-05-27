// SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "aurorawaylandextsessionlockv1_p.h"
#include "aurorawaylandextsessionlockv1integration_p.h"

namespace Aurora {

namespace Compositor {

namespace Internal {

WaylandExtSessionLockV1Integration::WaylandExtSessionLockV1Integration(WaylandQuickShellSurfaceItem *item)
    : WaylandQuickShellIntegration(item)
    , m_lockSurface(qobject_cast<WaylandExtSessionLockSurfaceV1 *>(item->shellSurface()))
{
    item->setSurface(m_lockSurface->surface());
    item->setOutput(m_lockSurface->output());
}

} // nmamespace Internal

} // namespace Compositor

} // namespace Aurora
