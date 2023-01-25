// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDQTTEXTINPUTMETHODMANAGER_P_H
#define AURORA_COMPOSITOR_WAYLANDQTTEXTINPUTMETHODMANAGER_P_H

#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>

#include <LiriAuroraCompositor/private/aurora-server-qt-text-input-method-unstable-v1.h>

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

class LIRIAURORACOMPOSITOR_EXPORT WaylandQtTextInputMethodManagerPrivate : public WaylandCompositorExtensionPrivate, public PrivateServer::qt_text_input_method_manager_v1
{
    Q_DECLARE_PUBLIC(WaylandQtTextInputMethodManager)
public:
    WaylandQtTextInputMethodManagerPrivate();

protected:
    void text_input_method_manager_v1_get_text_input_method(Resource *resource, uint32_t id, struct ::wl_resource *seat) override;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDQTTEXTINPUTMETHODMANAGER_P_H
