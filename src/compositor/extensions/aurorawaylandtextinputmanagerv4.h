// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandCompositorExtension>

#include <QtCore/QSize>

namespace Aurora {

namespace Compositor {

class WaylandTextInputManagerV4Private;

class LIRIAURORACOMPOSITOR_EXPORT WaylandTextInputManagerV4 : public WaylandCompositorExtensionTemplate<WaylandTextInputManagerV4>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandTextInputManagerV4)
public:
    WaylandTextInputManagerV4();
    explicit WaylandTextInputManagerV4(WaylandCompositor *compositor);
    ~WaylandTextInputManagerV4() override;

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
};

} // namespace Compositor

} // namespace Aurora

