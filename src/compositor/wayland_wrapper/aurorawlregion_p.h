// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Aurora API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>

#include <QRegion>

#include <wayland-util.h>
#include <LiriAuroraCompositor/private/aurora-server-wayland.h>

namespace Aurora {

namespace Compositor {

namespace Internal {

class LIRIAURORACOMPOSITOR_EXPORT Region : public PrivateServer::wl_region
{
public:
    Region(struct wl_client *client, uint32_t id);
    ~Region() override;

    static Region *fromResource(struct ::wl_resource *resource);

    uint id() const { return wl_resource_get_id(resource()->handle); }

    QRegion region() const { return m_region; }

private:
    Q_DISABLE_COPY(Region)

    QRegion m_region;

    void region_destroy_resource(Resource *) override;

    void region_destroy(Resource *resource) override;
    void region_add(Resource *resource, int32_t x, int32_t y, int32_t w, int32_t h) override;
    void region_subtract(Resource *resource, int32_t x, int32_t y, int32_t w, int32_t h) override;
};

}

} // namespace Compositor

} // namespace Aurora


