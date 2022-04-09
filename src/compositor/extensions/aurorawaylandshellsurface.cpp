/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
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

#include <LiriAuroraCompositor/WaylandShellSurface>

/*!
 * \class WaylandShellSurfaceTemplate
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief WaylandShellSurfaceTemplate is a convenience class for creating custom shell surface
 * classes.
 *
 * WaylandShellSurfaceTemplate is a template class which inherits from WaylandShellSurface and
 * is convenience for building custom shell extensions.
 *
 * It provides the connection between Qt Wayland Compositor and the class generated by
 * \c qtwaylandscanner, based on the XML description of the extension protocol.
 *
 * It provides two specific pieces of convenience:
 * \list
 *   \li A reimplementation of \l{WaylandCompositorExtension::extensionInterface()} which returns
 *   the \c wl_interface pointer for the qtwaylandscanner-generated base class.
 *   \li A static \l{findIn()} function which searches for an instance of the extension in a
 *   provided container, and returns this if it is found.
 * \endlist
 *
 * The same usage pattern applies as for WaylandCompositorExtensionTemplate.
 *
 * \sa {Qt Wayland Compositor Examples - Custom Shell}
 */

/*!
 *  \fn template <typename T> T *WaylandShellSurfaceTemplate<T>::findIn(WaylandObject *container)
 *
 *  If any instance of the interface has been registered with \a container, this is returned.
 *  Otherwise null is returned. The look-up is based on the generated \c interfaceName() which
 *  matches the interface name in the protocol description.
 */


/*!
 * \qmltype ShellSurface
 * \instantiates WaylandShellSurface
 * \inqmlmodule Aurora.Compositor
 * \since 5.8
 * \brief Provides a common interface for surface roles specified by shell extensions.
 *
 * This interface represents a Wayland surface role given by a Wayland protocol extension that
 * defines how the WaylandSurface should map onto the screen.
 *
 * \note Even though this type contains a very limited API, the properties and signals of the
 * implementations are named consistently. For example, if you're only using desktop shell
 * extensions in your compositor, it's safe to access properties such as title, maximized, etc.
 * directly on the ShellSurface. See the various implementations for additional properties and
 * signals.
 *
 * \sa WaylandSurface, ShellSurfaceItem, WlShellSurface, IviSurface
 */

/*!
 * \class WaylandShellSurface
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The WaylandShellSurface class is a common interface for surface roles specified by shell extensions.
 *
 * This interface represents a Wayland surface role given by a Wayland protocol extension that
 * defines how the WaylandSurface should map onto the screen.
 *
 * \sa WaylandSurface, WaylandWlShellSurface, WaylandIviSurface, WaylandShellSurfaceTemplate
 */

#if LIRI_FEATURE_aurora_compositor_quick
/*!
 * \fn WaylandQuickShellIntegration *WaylandShellSurface::createIntegration(WaylandQuickShellSurfaceItem *item)
 *
 * Creates a WaylandQuickShellIntegration for this WaylandQuickShellSurface. It's called
 * automatically when \a {item}'s \l {WaylandQuickShellSurfaceItem::}{shellSurface} is assigned.
 *
 * \sa WaylandQuickShellSurfaceItem
 */
#endif

/*!
 * \qmlproperty enum QtWaylandCompositor::ShellSurface::windowType
 *
 * This property holds the window type of the ShellSurface.
 */

/*!
 * \property WaylandShellSurface::windowType
 *
 * This property holds the window type of the WaylandShellSurface.
 */
