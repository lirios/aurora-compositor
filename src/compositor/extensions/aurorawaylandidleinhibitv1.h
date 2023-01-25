// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDIDLEINHIBITV1_H
#define AURORA_COMPOSITOR_WAYLANDIDLEINHIBITV1_H

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

private:
    QScopedPointer<WaylandIdleInhibitManagerV1Private> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDIDLEINHIBITV1_H
