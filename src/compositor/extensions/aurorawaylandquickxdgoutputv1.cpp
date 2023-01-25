// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandOutput>

#include "aurorawaylandquickxdgoutputv1.h"
#include "aurorawaylandxdgoutputv1_p.h"

namespace Aurora {

namespace Compositor {

WaylandQuickXdgOutputV1::WaylandQuickXdgOutputV1()
    : WaylandXdgOutputV1()
{
}

void WaylandQuickXdgOutputV1::componentComplete()
{
    // Try to find the manager from the compositor extensions
    if (!manager()) {
        for (auto *p = parent(); p != nullptr; p = p->parent()) {
            if (auto *c = qobject_cast<WaylandCompositor *>(p)) {
                for (auto *extension : c->extensions()) {
                    if (auto *m = qobject_cast<WaylandXdgOutputManagerV1 *>(extension)) {
                        WaylandXdgOutputV1Private::get(this)->setManager(m);
                        break;
                    }
                }
            }
        }
    }

    // Try to find the output from the parents
    if (!output()) {
        for (auto *p = parent(); p != nullptr; p = p->parent()) {
            if (auto *o = qobject_cast<WaylandOutput *>(p)) {
                WaylandXdgOutputV1Private::get(this)->setOutput(o);
                break;
            }
        }
    }
}

} // namespace Compositor

} // namespace Aurora
