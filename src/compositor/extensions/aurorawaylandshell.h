// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDSHELL_H
#define AURORA_COMPOSITOR_WAYLANDSHELL_H

#include <LiriAuroraCompositor/aurorawaylandqmlinclude.h>
#include <LiriAuroraCompositor/aurorawaylandcompositorextension.h>

namespace Aurora {

namespace Compositor {

class WaylandShellPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandShell : public WaylandCompositorExtension
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandShell)
    Q_PROPERTY(FocusPolicy focusPolicy READ focusPolicy WRITE setFocusPolicy NOTIFY focusPolicyChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(Shell)
    QML_UNCREATABLE("")
    QML_ADDED_IN_VERSION(1, 0)
#endif
public:
    enum FocusPolicy {
        AutomaticFocus,
        ManualFocus
    };
    Q_ENUM(FocusPolicy)

    WaylandShell();
    WaylandShell(WaylandObject *waylandObject);
    ~WaylandShell();

    FocusPolicy focusPolicy() const;
    void setFocusPolicy(FocusPolicy focusPolicy);

Q_SIGNALS:
    void focusPolicyChanged();

private:
    QScopedPointer<WaylandShellPrivate> const d_ptr;
};

template <typename T>
class LIRIAURORACOMPOSITOR_EXPORT WaylandShellTemplate : public WaylandShell
{
public:
    WaylandShellTemplate()
        : WaylandShell()
    { }

    WaylandShellTemplate(WaylandObject *container)
        : WaylandShell(container)
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

#endif // AURORA_COMPOSITOR_WAYLANDSHELL_H
