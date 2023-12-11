// Copyright (C) 2020 The Qt Company Ltd.
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

#include <LiriAuroraCompositor/private/aurora-server-touch-extension.h>
#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandCompositorExtensionTemplate>
#include <QtCore/private/qglobal_p.h>

#include <wayland-util.h>

class QTouchEvent;

namespace Aurora {

namespace Compositor {

class Surface;
class WaylandView;

namespace Internal {

class TouchExtensionGlobal : public WaylandCompositorExtensionTemplate<TouchExtensionGlobal>, public PrivateServer::qt_touch_extension
{
    Q_OBJECT
    Q_PROPERTY(BehaviorFlags behaviorFlags READ behaviorFlags WRITE setBehviorFlags NOTIFY behaviorFlagsChanged)
public:

    enum BehaviorFlag{
        None = 0x00,
        MouseFromTouch = 0x01
    };
    Q_DECLARE_FLAGS(BehaviorFlags, BehaviorFlag)

    TouchExtensionGlobal(WaylandCompositor *compositor);
    ~TouchExtensionGlobal() override;

    bool postTouchEvent(QTouchEvent *event, WaylandSurface *surface);

    void setBehviorFlags(BehaviorFlags flags);
    BehaviorFlags behaviorFlags() const { return m_flags; }

Q_SIGNALS:
    void behaviorFlagsChanged();

protected:
    void touch_extension_bind_resource(Resource *resource) override;
    void touch_extension_destroy_resource(Resource *resource) override;

private:
    WaylandCompositor *m_compositor = nullptr;
    BehaviorFlags m_flags = BehaviorFlag::None;
    QList<Resource *> m_resources;
    QList<float> m_posData;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TouchExtensionGlobal::BehaviorFlags)

}

} // namespace Compositor

} // namespace Aurora

