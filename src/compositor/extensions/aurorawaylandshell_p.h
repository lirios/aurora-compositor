// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDSHELL_P_H
#define AURORA_COMPOSITOR_WAYLANDSHELL_P_H

#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/WaylandShell>

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

class LIRIAURORACOMPOSITOR_EXPORT WaylandShellPrivate : public WaylandCompositorExtensionPrivate
{
    Q_DECLARE_PUBLIC(WaylandShell)
public:
    WaylandShellPrivate(WaylandShell *self);

    WaylandShell::FocusPolicy focusPolicy = WaylandShell::AutomaticFocus;

protected:
    WaylandShell *q_ptr = nullptr;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDSHELL_P_H
