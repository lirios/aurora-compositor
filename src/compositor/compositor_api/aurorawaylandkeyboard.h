/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
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

#ifndef AURORA_COMPOSITOR_WAYLANDKEYBOARD_H
#define AURORA_COMPOSITOR_WAYLANDKEYBOARD_H

#include <QtCore/QObject>

#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/WaylandSurface>

namespace Aurora {

namespace Compositor {

class WaylandKeyboard;
class WaylandKeyboardPrivate;
class WaylandSeat;
class WaylandKeymap;

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandKeyboard : public WaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandKeyboard)
    Q_PROPERTY(quint32 repeatRate READ repeatRate WRITE setRepeatRate NOTIFY repeatRateChanged)
    Q_PROPERTY(quint32 repeatDelay READ repeatDelay WRITE setRepeatDelay NOTIFY repeatDelayChanged)
public:
    WaylandKeyboard(WaylandSeat *seat, QObject *parent = nullptr);

    WaylandSeat *seat() const;
    WaylandCompositor *compositor() const;

    quint32 repeatRate() const;
    void setRepeatRate(quint32 rate);

    quint32 repeatDelay() const;
    void setRepeatDelay(quint32 delay);

    virtual void setFocus(WaylandSurface *surface);

    virtual void sendKeyModifiers(WaylandClient *client, uint32_t serial);
    virtual void sendKeyPressEvent(uint code);
    virtual void sendKeyReleaseEvent(uint code);

    WaylandSurface *focus() const;
    WaylandClient *focusClient() const;

    virtual void addClient(WaylandClient *client, uint32_t id, uint32_t version);

    uint keyToScanCode(int qtKey) const;

Q_SIGNALS:
    void focusChanged(Aurora::Compositor::WaylandSurface *surface);
    void repeatRateChanged(quint32 repeatRate);
    void repeatDelayChanged(quint32 repeatDelay);

private:
    void focusDestroyed(void *data);

private Q_SLOTS:
    void updateKeymap();
};

} // namespace Compositor

} // namespace Aurora

#endif //QWAYLANDKEYBOARD_H
