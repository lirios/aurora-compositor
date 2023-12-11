// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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
#include <LiriAuroraCompositor/aurorawaylandinputmethodcontrol.h>

#include <QtCore/private/qobject_p.h>

namespace Aurora {

namespace Compositor {

class WaylandCompositor;
class WaylandSeat;
class WaylandSurface;
class WaylandTextInput;
class WaylandTextInputV3;
class WaylandQtTextInputMethod;

class LIRIAURORACOMPOSITOR_EXPORT WaylandInputMethodControlPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(WaylandInputMethodControl)

public:
    explicit WaylandInputMethodControlPrivate(WaylandSurface *surface);

    WaylandTextInput *textInput() const;
    WaylandTextInputV3 *textInputV3() const;
    WaylandQtTextInputMethod *textInputMethod() const;

    WaylandCompositor *compositor = nullptr;
    WaylandSeat *seat = nullptr;
    WaylandSurface *surface = nullptr;
    bool enabled = false;
};

} // namespace Compositor

} // namespace Aurora

