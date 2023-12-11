// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QtCore/QObject>
#include <LiriAuroraCompositor/liriauroracompositorglobal.h>

struct wl_resource;

namespace Aurora {

namespace Compositor {

class WaylandDestroyListenerPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandDestroyListener : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandDestroyListener)
public:
    WaylandDestroyListener(QObject *parent = nullptr);
    void listenForDestruction(struct wl_resource *resource);
    void reset();

Q_SIGNALS:
    void fired(void *data);

};

} // namespace Compositor

} // namespace Aurora

