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

#ifndef AURORA_COMPOSITOR_WAYLANDSHELLSURFACE_H
#define AURORA_COMPOSITOR_WAYLANDSHELLSURFACE_H

#include <LiriAuroraCompositor/aurorawaylandqmlinclude.h>
#include <LiriAuroraCompositor/aurorawaylandcompositorextension.h>

namespace Aurora {

namespace Compositor {

class WaylandQuickShellIntegration;
class WaylandQuickShellSurfaceItem;

class LIRIAURORACOMPOSITOR_EXPORT WaylandShellSurface : public WaylandCompositorExtension
{
    Q_OBJECT
    Q_PROPERTY(Qt::WindowType windowType READ windowType NOTIFY windowTypeChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(ShellSurface)
    QML_UNCREATABLE("")
    QML_ADDED_IN_VERSION(1, 0)
#endif
public:
#if LIRI_FEATURE_aurora_compositor_quick
    virtual WaylandQuickShellIntegration *createIntegration(WaylandQuickShellSurfaceItem *item) = 0;
#endif
    WaylandShellSurface(WaylandObject *waylandObject) : WaylandCompositorExtension(waylandObject) {}
    virtual Qt::WindowType windowType() const { return Qt::WindowType::Window; }

Q_SIGNALS:
    void windowTypeChanged();
};

template <typename T>
class LIRIAURORACOMPOSITOR_EXPORT WaylandShellSurfaceTemplate : public WaylandShellSurface
{
public:
    WaylandShellSurfaceTemplate(WaylandObject *container = nullptr)
        : WaylandShellSurface(container)
    { }

    const struct wl_interface *extensionInterface() const override
    {
        return T::interface();
    }

    static T *findIn(WaylandObject *container)
    {
        if (!container) return nullptr;
        return qobject_cast<T *>(container->extension(T::interfaceName()));
    }
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDSHELLSURFACE_H
