// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDINPUTMETHODCONTROL_P_H
#define AURORA_COMPOSITOR_WAYLANDINPUTMETHODCONTROL_P_H

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
#include <LiriAuroraCompositor/aurorawaylandinputmethodcontrol.h>

namespace Aurora {

namespace Compositor {

class WaylandCompositor;
class WaylandSeat;
class WaylandSurface;
class WaylandTextInput;
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
class WaylandTextInputV4;
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
class WaylandQtTextInputMethod;

class LIRIAURORACOMPOSITOR_EXPORT WaylandInputMethodControlPrivate
{
    Q_DECLARE_PUBLIC(WaylandInputMethodControl)

public:
    explicit WaylandInputMethodControlPrivate(WaylandInputMethodControl *self, WaylandSurface *surface);

    WaylandTextInput *textInput() const;
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
    WaylandTextInputV4 *textInputV4() const;
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
    WaylandQtTextInputMethod *textInputMethod() const;

    WaylandCompositor *compositor = nullptr;
    WaylandSeat *seat = nullptr;
    WaylandSurface *surface = nullptr;
    bool enabled = false;

private:
    WaylandInputMethodControl *q_ptr = nullptr;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDINPUTMETHODCONTROL_P_H
