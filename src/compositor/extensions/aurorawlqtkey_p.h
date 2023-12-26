// Copyright (C) 2019 The Qt Company Ltd.
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

#include <LiriAuroraCompositor/WaylandCompositorExtensionTemplate>
#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/private/aurora-server-qt-key-unstable-v1.h>
#include <QtCore/private/qglobal_p.h>

#include <wayland-util.h>

class QKeyEvent;

namespace Aurora {

namespace Compositor {

class WaylandSurface;

namespace Internal {

class QtKeyExtensionGlobal : public WaylandCompositorExtensionTemplate<QtKeyExtensionGlobal>, public PrivateServer::zqt_key_v1
{
    Q_OBJECT
public:
    QtKeyExtensionGlobal(WaylandCompositor *compositor);

    bool postQtKeyEvent(QKeyEvent *event, WaylandSurface *surface);

private:
    WaylandCompositor *m_compositor = nullptr;
};

}

} // namespace Compositor

} // namespace Aurora

