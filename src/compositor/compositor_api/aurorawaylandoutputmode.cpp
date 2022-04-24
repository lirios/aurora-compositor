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

#include "aurorawaylandoutputmode.h"
#include "aurorawaylandoutputmode_p.h"

/*!
   \class WaylandOutputMode
   \inmodule AuroraCompositor
   \since 5.8
   \brief The WaylandOutputMode class holds the resolution and refresh rate of an output.

   WaylandOutputMode holds the resolution and refresh rate of an output.
   Resolution is expressed in pixels and refresh rate is measured in mHz.

   \sa WaylandOutput
*/

namespace Aurora {

namespace Compositor {

WaylandOutputMode::WaylandOutputMode()
    : d(new WaylandOutputModePrivate)
{
}

WaylandOutputMode::WaylandOutputMode(const QSize &size, int refreshRate)
    : d(new WaylandOutputModePrivate)
{
    d->size = size;
    d->refreshRate = refreshRate;
}

WaylandOutputMode::WaylandOutputMode(const WaylandOutputMode &other)
    : d(new WaylandOutputModePrivate)
{
    d->size = other.size();
    d->refreshRate = other.refreshRate();
}

WaylandOutputMode::~WaylandOutputMode()
{
    delete d;
}

WaylandOutputMode &WaylandOutputMode::operator=(const WaylandOutputMode &other)
{
    d->size = other.size();
    d->refreshRate = other.refreshRate();
    return *this;
}

/*!
    Returns \c true if this mode is equal to \a other,
    otherwise returns \c false.
*/
bool WaylandOutputMode::operator==(const WaylandOutputMode &other) const
{
    return size() == other.size() && refreshRate() == other.refreshRate();
}

/*!
    Returns \c true if this mode is not equal to \a other,
    otherwise returns \c false.
*/
bool WaylandOutputMode::operator!=(const WaylandOutputMode &other) const
{
    return size() != other.size() || refreshRate() != other.refreshRate();
}

/*!
    Returns whether this mode contains a valid resolution and refresh rate.
*/
bool WaylandOutputMode::isValid() const
{
    return !d->size.isEmpty() && d->refreshRate > 0;
}

/*!
    Returns the resolution in pixels.
*/
QSize WaylandOutputMode::size() const
{
    return d->size;
}

/*!
    Returns the refresh rate in mHz.
*/
int WaylandOutputMode::refreshRate() const
{
    return d->refreshRate;
}

/*!
 * \internal
 */
void WaylandOutputMode::setSize(const QSize &size)
{
    d->size = size;
}

} // namespace Compositor

} // namespace Aurora
