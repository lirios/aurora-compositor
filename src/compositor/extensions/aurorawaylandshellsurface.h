// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDSHELLSURFACE_H
#define AURORA_COMPOSITOR_WAYLANDSHELLSURFACE_H

#include <LiriAuroraCompositor/aurorawaylandqmlinclude.h>
#include <LiriAuroraCompositor/aurorawaylandcompositorextension.h>

namespace Aurora {

namespace Compositor {

class WaylandQuickShellIntegration;
class WaylandQuickShellSurfaceItem;
class WaylandShell;

class LIRIAURORACOMPOSITOR_EXPORT WaylandShellSurface : public WaylandCompositorExtension
{
    Q_OBJECT
    Q_PROPERTY(Qt::WindowType windowType READ windowType NOTIFY windowTypeChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandShell *shell READ shell NOTIFY shellChanged)
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

    virtual WaylandShell *shell() const = 0;

    virtual Qt::WindowType windowType() const { return Qt::WindowType::Window; }

protected:
    WaylandShellSurface(WaylandCompositorExtensionPrivate &dd);
    WaylandShellSurface(WaylandObject *container, WaylandCompositorExtensionPrivate &dd);

Q_SIGNALS:
    void shellChanged();
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
        : WaylandShellSurface(container, dd)
    { }
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDSHELLSURFACE_H
