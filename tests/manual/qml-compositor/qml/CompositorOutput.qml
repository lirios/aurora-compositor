// SPDX-FileCopyrightText: 2021 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
//
// SPDX-License-Identifier: BSD-3-Clause

import QtQuick 2.15
import QtQuick.Window 2.15
import Aurora.Compositor 1.0

WaylandOutput {
    id: output

    property ListModel shellSurfaces: ListModel {}
    property bool isNestedCompositor: Qt.platform.pluginName.startsWith("wayland") || Qt.platform.pluginName === "xcb"

    function handleShellSurface(shellSurface) {
        shellSurfaces.append({shellSurface: shellSurface});
    }

    // During development, it can be useful to start the compositor inside X11 or
    // another Wayland compositor. In such cases, set sizeFollowsWindow to true to
    // enable resizing of the compositor window to be forwarded to the Wayland clients
    // as the output (screen) changing resolution. Consider setting it to false if you
    // are running the compositor using eglfs, linuxfb or similar QPA backends.
    sizeFollowsWindow: output.isNestedCompositor

    window: Window {
        width: 1024
        height: 760
        color: "gainsboro"
        visible: true

        WaylandMouseTracker {
            id: mouseTracker

            anchors.fill: parent

            // Set this to false to disable the outer mouse cursor when running nested
            // compositors. Otherwise you would see two mouse cursors, one for each compositor.
            windowSystemCursorEnabled: !clientCursor.visible

            Repeater {
                model: output.shellSurfaces
                // Chrome displays a shell surface on the screen (See Chrome.qml)
                Chrome {
                    shellSurface: modelData
                    onDestroyAnimationFinished: output.shellSurfaces.remove(index)
                }
            }

            // Draws the mouse cursor for a given Wayland seat
            WaylandCursorItem {
                id: clientCursor

                x: mouseTracker.mouseX
                y: mouseTracker.mouseY
                seat: output.compositor.defaultSeat
                visible: surface !== null && mouseTracker.containsMouse
            }
        }

        Shortcut {
            sequence: "Ctrl+Alt+Backspace"
            onActivated: Qt.quit()
        }
    }
}
