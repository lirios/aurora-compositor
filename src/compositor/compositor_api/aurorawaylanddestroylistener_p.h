// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Aurora API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <LiriAuroraCompositor/WaylandDestroyListener>

#include <QtCore/private/qobject_p.h>

#include <wayland-server-core.h>

namespace Aurora {

namespace Compositor {

class WaylandDestroyListenerPrivate : public QObjectPrivate
{
public:
    Q_DECLARE_PUBLIC(WaylandDestroyListener)

    WaylandDestroyListenerPrivate();

    static void handler(wl_listener *listener, void *data);

    struct Listener {
        wl_listener listener;
        WaylandDestroyListenerPrivate *parent = nullptr;
    };
    Listener listener;
};

} // namespace Compositor

} // namespace Aurora

