// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDQTTEXTINPUTMETHODMANAGER_H
#define AURORA_COMPOSITOR_WAYLANDQTTEXTINPUTMETHODMANAGER_H

#include <LiriAuroraCompositor/WaylandCompositorExtension>

#include <QtCore/QSize>

namespace Aurora {

namespace Compositor {

class WaylandQtTextInputMethodManagerPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandQtTextInputMethodManager : public WaylandCompositorExtensionTemplate<WaylandQtTextInputMethodManager>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandQtTextInputMethodManager)
public:
    WaylandQtTextInputMethodManager();
    WaylandQtTextInputMethodManager(WaylandCompositor *compositor);

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDQTTEXTINPUTMETHODMANAGER_H
