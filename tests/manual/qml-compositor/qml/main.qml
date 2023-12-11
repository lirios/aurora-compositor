// SPDX-FileCopyrightText: 2021 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
//
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick 2.15
import Aurora.Compositor 1.0
import Aurora.Compositor.WlShell 1.0
import Aurora.Compositor.XdgShell 1.0

WaylandCompositor {
    id: waylandCompositor

    CompositorOutput {
        id: output

        compositor: waylandCompositor
    }

    WlShell {
        onWlShellSurfaceCreated: output.handleShellSurface(shellSurface)
    }

    XdgShell {
        onToplevelCreated: output.handleShellSurface(xdgSurface)
    }
}
