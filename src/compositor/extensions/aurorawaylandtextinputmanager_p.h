// Copyright (C) 2017-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDTEXTINPUTMANAGER_P_H
#define AURORA_COMPOSITOR_WAYLANDTEXTINPUTMANAGER_P_H

#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>

#include <LiriAuroraCompositor/private/aurora-server-text-input-unstable-v2.h>

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

class LIRIAURORACOMPOSITOR_EXPORT WaylandTextInputManagerPrivate : public WaylandCompositorExtensionPrivate, public PrivateServer::zwp_text_input_manager_v2
{
    Q_DECLARE_PUBLIC(WaylandTextInputManager)
public:
    WaylandTextInputManagerPrivate(WaylandTextInputManager *self);

protected:
    void zwp_text_input_manager_v2_get_text_input(Resource *resource, uint32_t id, struct ::wl_resource *seatResource) override;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDTEXTINPUTMANAGER_P_H
