/****************************************************************************
**
** Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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
