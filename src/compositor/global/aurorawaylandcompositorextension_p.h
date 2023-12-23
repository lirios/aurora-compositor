// Copyright (C) 2017 The Qt Company Ltd.
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

#include <LiriAuroraCompositor/WaylandCompositorExtension>

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandCompositorExtensionPrivate
{
    Q_DECLARE_PUBLIC(WaylandCompositorExtension)
public:
    WaylandCompositorExtensionPrivate(WaylandCompositorExtension *self)
        : q_ptr(self)
    {
    }

    static WaylandCompositorExtensionPrivate *get(WaylandCompositorExtension *extension) { return extension->d_func(); }

    WaylandObject *extension_container = nullptr;
    bool initialized = false;

protected:
    WaylandCompositorExtension *q_ptr = nullptr;
};

} // namespace Compositor

} // namespace Aurora

