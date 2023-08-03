// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QWindow>

#include <LiriAuroraPlatform/liriauroraplatformglobal.h>

namespace Aurora {

namespace Platform {

class Output;
class WindowPrivate;

class LIRIAURORAPLATFORM_EXPORT Window : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Output *output READ output CONSTANT)
    Q_PROPERTY(QWindow *qtWindow READ qtWindow CONSTANT)
    Q_DECLARE_PRIVATE(Window)
public:
    ~Window();

    Output *output() const;
    QWindow *qtWindow() const;

    virtual void *resource(const QByteArray &name);

    virtual bool create() = 0;
    virtual void destroy() = 0;

    virtual void changeCursor(QCursor *cursor);

protected:
    explicit Window(Output *output, QWindow *qtWindow, QObject *parent = nullptr);

    QScopedPointer<WindowPrivate> const d_ptr;
};

} // namespace Platform

} // namespace Aurora
