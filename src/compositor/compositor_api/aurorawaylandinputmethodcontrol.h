// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDINPUTMETHODCONTROL_H
#define AURORA_COMPOSITOR_WAYLANDINPUTMETHODCONTROL_H

#include <QtGui/qtguiglobal.h>
#include <QtCore/QObject>

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
