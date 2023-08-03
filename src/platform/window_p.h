// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QPointer>

#include <LiriAuroraPlatform/Window>

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

namespace Platform {

class Output;

class LIRIAURORAPLATFORM_EXPORT WindowPrivate
{
    Q_DECLARE_PUBLIC(Window)
public:
    explicit WindowPrivate(Window *self);

    static WindowPrivate *get(Window *window)
    {
        return window->d_func();
    }

    QPointer<Output> output;
    QPointer<QWindow> qtWindow;

protected:
    Window *q_ptr = nullptr;
};

} // namespace Platform

} // namespace Aurora
