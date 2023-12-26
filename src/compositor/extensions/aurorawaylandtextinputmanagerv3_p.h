// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>

#include <LiriAuroraCompositor/private/aurora-server-text-input-unstable-v3.h>

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

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandTextInputManagerV3Private : public WaylandCompositorExtensionPrivate, public PrivateServer::zwp_text_input_manager_v3
{
    Q_DECLARE_PUBLIC(WaylandTextInputManagerV3)
public:
    WaylandTextInputManagerV3Private();

protected:
    void zwp_text_input_manager_v3_get_text_input(Resource *resource, uint32_t id, struct ::wl_resource *seatResource) override;
};

} // namespace Compositor

} // namespace Aurora

