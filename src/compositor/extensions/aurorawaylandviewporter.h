// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandCompositorExtension>

namespace Aurora {

namespace Compositor {

class WaylandViewporterPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandViewporter
        : public WaylandCompositorExtensionTemplate<WaylandViewporter>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandViewporter)

public:
    explicit WaylandViewporter();
    explicit WaylandViewporter(WaylandCompositor *compositor);

    void initialize() override;

    static const struct wl_interface *interface();
};

} // namespace Compositor

} // namespace Aurora

