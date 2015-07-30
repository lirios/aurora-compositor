/****************************************************************************
**
** Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QTWAYLAND_QWLINPUTMETHOD_H
#define QTWAYLAND_QWLINPUTMETHOD_H

#include <QtCompositor/private/qwayland-server-input-method.h>

#include <QObject>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QWaylandInputDevice;
class QWaylandCompositor;

namespace QtWayland {

class InputMethodContext;
class TextInput;
class Surface;

class InputMethod : public QObject, public QtWaylandServer::wl_input_method
{
    Q_OBJECT

public:
    explicit InputMethod(QWaylandCompositor *compositor, QWaylandInputDevice *seat);
    ~InputMethod();

    void activate(TextInput *textInput);
    void deactivate();

    bool isBound() const;

    InputMethodContext *context() const;
    TextInput *textInput() const;

protected:
    void input_method_bind_resource(Resource *resource);
    void input_method_destroy_resource(Resource *resource);

private Q_SLOTS:
    void focusChanged(Surface *surface);

private:
    QWaylandCompositor *m_compositor;
    QWaylandInputDevice *m_seat;
    Resource *m_resource;
    TextInput *m_textInput;
    InputMethodContext *m_context;
};

} // namespace QtWayland

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLINPUTMETHOD_H
