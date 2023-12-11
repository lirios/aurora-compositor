// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/auroraqmlinclude.h>
#include <LiriAuroraCompositor/aurorawaylandcompositorextension.h>

namespace Aurora {

namespace Compositor {

class WaylandShellPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandShell : public WaylandCompositorExtension
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandShell)
    Q_PROPERTY(FocusPolicy focusPolicy READ focusPolicy WRITE setFocusPolicy NOTIFY focusPolicyChanged)

    QML_NAMED_ELEMENT(Shell)
    QML_UNCREATABLE("")
    QML_ADDED_IN_VERSION(1, 0)
public:
    enum FocusPolicy {
        AutomaticFocus,
        ManualFocus
    };
    Q_ENUM(FocusPolicy)

    WaylandShell();
    WaylandShell(WaylandObject *waylandObject);

    FocusPolicy focusPolicy() const;
    void setFocusPolicy(FocusPolicy focusPolicy);

Q_SIGNALS:
    void focusPolicyChanged();

protected:
    explicit WaylandShell(WaylandShellPrivate &dd);
    explicit WaylandShell(WaylandObject *container, WaylandShellPrivate &dd);
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

protected:
    WaylandShellTemplate(WaylandShellPrivate &dd)
        : WaylandShell(dd)
    { }

    WaylandShellTemplate(WaylandObject *container, WaylandShellPrivate &dd)
        : WaylandShell(container,dd)
    { }
};

} // namespace Compositor

} // namespace Aurora

