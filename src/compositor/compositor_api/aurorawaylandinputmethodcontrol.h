/****************************************************************************
**
** Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef AURORA_COMPOSITOR_WAYLANDINPUTMETHODCONTROL_H
#define AURORA_COMPOSITOR_WAYLANDINPUTMETHODCONTROL_H

#include <QtGui/qtguiglobal.h>
#include <QObject>

class QInputMethodEvent;

namespace Aurora {

namespace Compositor {

class WaylandCompositor;
class WaylandInputMethodControlPrivate;
class WaylandSurface;
class WaylandTextInput;

class WaylandInputMethodControl : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandInputMethodControl)
    Q_DISABLE_COPY(WaylandInputMethodControl)

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
public:
    explicit WaylandInputMethodControl(WaylandSurface *surface);
    ~WaylandInputMethodControl();

    QVariant inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const;

    void inputMethodEvent(QInputMethodEvent *event);

    bool enabled() const;
    void setEnabled(bool enabled);

    void setSurface(WaylandSurface *surface);
    void updateTextInput();

Q_SIGNALS:
    void enabledChanged(bool enabled);
    void updateInputMethod(Qt::InputMethodQueries queries);

private:
    QScopedPointer<WaylandInputMethodControlPrivate> const d_ptr;

    void defaultSeatChanged();
    void surfaceEnabled(WaylandSurface *surface);
    void surfaceDisabled(WaylandSurface *surface);
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDINPUTMETHODCONTROL_H
