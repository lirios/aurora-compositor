// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "output.h"
#include "window.h"
#include "window_p.h"

namespace Aurora {

namespace Platform {

Window::Window(Output *output, QWindow *qtWindow, QObject *parent)
    : QObject(parent)
    , d_ptr(new WindowPrivate(this))
{
    d_ptr->output = output;
    d_ptr->qtWindow = qtWindow;
}

Window::~Window()
{
}

Output *Window::output() const
{
    Q_D(const Window);
    return d->output;
}

QWindow *Window::qtWindow() const
{
    Q_D(const Window);
    return d->qtWindow;
}

void *Window::resource(const QByteArray &name)
{
    Q_UNUSED(name)
    return nullptr;
}

void Window::changeCursor(QCursor *cursor)
{
    Q_UNUSED(cursor)
}

/*
 * WindowPrivate
 */

WindowPrivate::WindowPrivate(Window *self)
    : q_ptr(self)
{
}

} // namespace Platform

} // namespace Aurora
