// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandshell.h"
#include "aurorawaylandshell_p.h"

namespace Aurora {

namespace Compositor {

WaylandShellPrivate::WaylandShellPrivate(WaylandShell *self)
    : WaylandCompositorExtensionPrivate(self)
    , q_ptr(self)
{
}

WaylandShell::WaylandShell()
    : WaylandCompositorExtension()
    , d_ptr(new WaylandShellPrivate(this))
{
}

WaylandShell::WaylandShell(WaylandObject *waylandObject)
    : WaylandCompositorExtension(waylandObject)
    , d_ptr(new WaylandShellPrivate(this))
{
}

WaylandShell::~WaylandShell()
{
}

/*!
 * \enum WaylandShell::FocusPolicy
 *
 * This enum type is used to specify the focus policy for shell surfaces.
 *
 * \value AutomaticFocus Shell surfaces will automatically get keyboard focus when they are created.
 * \value ManualFocus The compositor will decide whether shell surfaces should get keyboard focus or not.
 */

/*!
 * \qmlproperty enumeration AuroraCompositor::Shell::focusPolicy
 *
 * This property holds the focus policy of the Shell.
 */

/*!
 * \property WaylandShell::focusPolicy
 *
 * This property holds the focus policy of the WaylandShell.
 */
WaylandShell::FocusPolicy WaylandShell::focusPolicy() const
{
    Q_D(const WaylandShell);
    return d->focusPolicy;
}

void WaylandShell::setFocusPolicy(WaylandShell::FocusPolicy focusPolicy)
{
    Q_D(WaylandShell);

    if (d->focusPolicy == focusPolicy)
        return;

    d->focusPolicy = focusPolicy;
    emit focusPolicyChanged();
}

} // namespace Compositor

} // namespace Aurora
