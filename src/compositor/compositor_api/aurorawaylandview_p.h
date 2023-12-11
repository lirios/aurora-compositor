// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include "aurorawaylandview.h"

#include <QtCore/QPoint>
#include <QtCore/QMutex>
#include <QtCore/private/qobject_p.h>

#include <LiriAuroraCompositor/WaylandBufferRef>

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

namespace Aurora {

namespace Compositor {

class WaylandSurface;
class WaylandOutput;

class WaylandViewPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(WaylandView)
public:
    static WaylandViewPrivate *get(WaylandView *view) { return view->d_func(); }

    WaylandViewPrivate()
    { }

    void markSurfaceAsDestroyed(WaylandSurface *surface);
    void setSurface(WaylandSurface *newSurface);
    void clearFrontBuffer();

    QObject *renderObject = nullptr;
    WaylandSurface *surface = nullptr;
    WaylandOutput *output = nullptr;
    QPointF requestedPos;
    QMutex bufferMutex;
    WaylandBufferRef currentBuffer;
    QRegion currentDamage;
    WaylandBufferRef nextBuffer;
    QRegion nextDamage;
    bool nextBufferCommitted = false;
    bool bufferLocked = false;
    bool broadcastRequestedPositionChanged = false;
    bool forceAdvanceSucceed = false;
    bool allowDiscardFrontBuffer = false;
    bool independentFrameCallback = false; //If frame callbacks are independent of the main quick scene graph
};

} // namespace Compositor

} // namespace Aurora

