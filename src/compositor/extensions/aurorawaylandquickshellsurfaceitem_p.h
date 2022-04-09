/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef AURORA_COMPOSITOR_WAYLANDQUICKSHELLSURFACEITEM_P_H
#define AURORA_COMPOSITOR_WAYLANDQUICKSHELLSURFACEITEM_P_H

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
    WaylandQuickShellSurfaceItemPrivate() {}
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

#endif // AURORA_COMPOSITOR_WAYLANDQUICKSHELLSURFACEITEM_P_H
