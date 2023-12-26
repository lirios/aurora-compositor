// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

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

