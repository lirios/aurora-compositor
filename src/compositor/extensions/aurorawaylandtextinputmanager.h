// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandCompositorExtension>

#include <QtCore/QSize>

namespace Aurora {

namespace Compositor {

class WaylandTextInputManagerPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandTextInputManager : public WaylandCompositorExtensionTemplate<WaylandTextInputManager>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandTextInputManager)
public:
    WaylandTextInputManager();
    WaylandTextInputManager(WaylandCompositor *compositor);
    ~WaylandTextInputManager();

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

private:
    QScopedPointer<WaylandTextInputManagerPrivate> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

