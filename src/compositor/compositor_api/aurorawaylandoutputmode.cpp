// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandoutputmode.h"
#include "aurorawaylandoutputmode_p.h"

namespace Aurora {

namespace Compositor {

/*!
   \class WaylandOutputMode
   \inmodule AuroraCompositor
   \since 5.8
   \brief The WaylandOutputMode class holds the resolution and refresh rate of an output.

   WaylandOutputMode holds the resolution and refresh rate of an output.
   Resolution is expressed in pixels and refresh rate is measured in mHz.

   \sa WaylandOutput
*/

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
