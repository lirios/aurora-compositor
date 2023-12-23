// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/WaylandXdgToplevel>

namespace Aurora {

namespace Compositor {

class WaylandXdgDecorationManagerV1Private;

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgDecorationManagerV1 : public WaylandCompositorExtensionTemplate<WaylandXdgDecorationManagerV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandXdgDecorationManagerV1)
    Q_PROPERTY(Aurora::Compositor::WaylandXdgToplevel::DecorationMode preferredMode READ preferredMode WRITE setPreferredMode NOTIFY preferredModeChanged)

public:
    explicit WaylandXdgDecorationManagerV1();
    ~WaylandXdgDecorationManagerV1();

    void initialize() override;

    WaylandXdgToplevel::DecorationMode preferredMode() const;
    void setPreferredMode(WaylandXdgToplevel::DecorationMode preferredMode);

    static const struct wl_interface *interface();

Q_SIGNALS:
    void preferredModeChanged();

private:
    QScopedPointer<WaylandXdgDecorationManagerV1Private> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

