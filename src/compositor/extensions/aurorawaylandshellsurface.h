// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/auroraqmlinclude.h>
#include <LiriAuroraCompositor/aurorawaylandcompositorextension.h>

namespace Aurora {

namespace Compositor {

class WaylandQuickShellIntegration;
class WaylandQuickShellSurfaceItem;

class LIRIAURORACOMPOSITOR_EXPORT WaylandShellSurface : public WaylandCompositorExtension
{
    Q_OBJECT
    Q_PROPERTY(Qt::WindowType windowType READ windowType NOTIFY windowTypeChanged)
    QML_NAMED_ELEMENT(ShellSurface)
    QML_UNCREATABLE("")
    QML_ADDED_IN_VERSION(1, 0)
public:
#if LIRI_FEATURE_aurora_compositor_quick
    virtual WaylandQuickShellIntegration *createIntegration(WaylandQuickShellSurfaceItem *item) = 0;
#endif
    WaylandShellSurface(WaylandObject *waylandObject) : WaylandCompositorExtension(waylandObject) {}
    virtual Qt::WindowType windowType() const { return Qt::WindowType::Window; }

protected:
    WaylandShellSurface(WaylandCompositorExtensionPrivate &dd) : WaylandCompositorExtension(dd){}
    WaylandShellSurface(WaylandObject *container, WaylandCompositorExtensionPrivate &dd) : WaylandCompositorExtension(container, dd) {}

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

protected:
    WaylandShellSurfaceTemplate(WaylandCompositorExtensionPrivate &dd)
        : WaylandShellSurface(dd)
    { }

    WaylandShellSurfaceTemplate(WaylandObject *container, WaylandCompositorExtensionPrivate &dd)
        : WaylandShellSurface(container,dd)
    { }
};

} // namespace Compositor

} // namespace Aurora

