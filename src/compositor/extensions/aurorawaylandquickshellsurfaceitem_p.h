// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandQuickShellSurfaceItem>
#include <LiriAuroraCompositor/WaylandQuickShellIntegration>
#include <LiriAuroraCompositor/private/aurorawaylandquickitem_p.h>
#include <QtCore/QBasicTimer>

#include <functional>

namespace Aurora {

namespace Compositor {

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

class WaylandShellSurface;
class WaylandQuickShellSurfaceItem;

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickShellSurfaceItemPrivate : public WaylandQuickItemPrivate
{
    Q_DECLARE_PUBLIC(WaylandQuickShellSurfaceItem)
public:
    WaylandQuickShellSurfaceItemPrivate(WaylandQuickShellSurfaceItem *self)
        : WaylandQuickItemPrivate(self)
        , q_ptr(self)
    {
    }

    WaylandQuickShellSurfaceItem *maybeCreateAutoPopup(WaylandShellSurface* shellSurface);
    static WaylandQuickShellSurfaceItemPrivate *get(WaylandQuickShellSurfaceItem *item) { return item->d_func(); }

    void raise() override;
    void lower() override;

    WaylandQuickShellIntegration *m_shellIntegration = nullptr;
    WaylandShellSurface *m_shellSurface = nullptr;
    QQuickItem *m_moveItem = nullptr;
    bool m_autoCreatePopupItems = true;
    bool staysOnTop = false;
    bool staysOnBottom = false;

private:
    WaylandQuickShellSurfaceItem *q_ptr = nullptr;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickShellEventFilter : public QObject
{
    Q_OBJECT
public:
    typedef std::function<void()> CallbackFunction;
    static void startFilter(WaylandClient *client, CallbackFunction closePopupCallback);
    static void cancelFilter();

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    void stopFilter();

    WaylandQuickShellEventFilter(QObject *parent = nullptr);
    bool eventFilter(QObject *, QEvent *) override;
    bool eventFilterInstalled = false;
    bool waitForRelease = false;
    QPointer<WaylandClient> client;
    CallbackFunction closePopups = nullptr;
    QBasicTimer mousePressTimeout;
    static WaylandQuickShellEventFilter *self;
};

} // namespace Compositor

} // namespace Aurora

