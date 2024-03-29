// Copyright (C) 2018 The Qt Company Ltd.
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

#include <QtCore/private/qglobal_p.h>

struct wl_resource;

namespace Aurora {

namespace Compositor {

namespace Internal {

template<typename return_type>
return_type fromResource(struct ::wl_resource *resource) {
    if (auto *r = std::remove_pointer<return_type>::type::Resource::fromResource(resource))
        return static_cast<return_type>(r->object());
    return nullptr;
}

} // namespace Internal

} // namespace Compositor

} // namespace Aurora

