// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawlqtkey_p.h"
#include <LiriAuroraCompositor/WaylandSurface>
#include <QKeyEvent>
#include <QWindow>

namespace Aurora {

namespace Compositor {

namespace Internal {

QtKeyExtensionGlobal::QtKeyExtensionGlobal(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate(compositor)
    , PrivateServer::zqt_key_v1(compositor->display(), 1)
    , m_compositor(compositor)
{
}

bool QtKeyExtensionGlobal::postQtKeyEvent(QKeyEvent *event, WaylandSurface *surface)
{
    uint32_t time = m_compositor->currentTimeMsecs();

    Resource *target = surface ? resourceMap().value(surface->waylandClient()) : 0;

    if (target) {
        send_key(target->handle,
                 surface ? surface->resource() : nullptr,
                 time, event->type(), event->key(), event->modifiers(),
                 event->nativeScanCode(),
                 event->nativeVirtualKey(),
                 event->nativeModifiers(),
                 event->text(),
                 event->isAutoRepeat(),
                 event->count());

        return true;
    }

    return false;
}

}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawlqtkey_p.cpp"
