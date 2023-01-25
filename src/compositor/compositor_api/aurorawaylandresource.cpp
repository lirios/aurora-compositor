// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandresource.h"

namespace Aurora {

namespace Compositor {

/*!
 * \class WaylandResource
 * \inmodule AuroraCompositor
 * \since 5.8
 * \brief WaylandResource is a container for a \c wl_resource.
 *
 * The WaylandResource is a simple wrapper around the Wayland type \c wl_resource, and makes it
 * possible to use wl_resource pointers in Qt Quick APIs.
 *
 * \sa {Qt Wayland Compositor Examples - Custom Shell}
 */

/*!
 * Constructs an invalid WaylandResource. The \l{resource()} accessor will return null.
 */
WaylandResource::WaylandResource()
{
}

/*!
 * Constructs a WaylandResource which contains \a resource.
 */
WaylandResource::WaylandResource(wl_resource *resource)
                : m_resource(resource)
{
}

/*!
 * \fn wl_resource *WaylandResource::resource() const
 *
 * \return the wl_resource pointer held by this WaylandResource.
 */

} // namespace Compositor

} // namespace Aurora
