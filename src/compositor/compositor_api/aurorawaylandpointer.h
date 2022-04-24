/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef AURORA_COMPOSITOR_WAYLANDPOINTER_H
#define AURORA_COMPOSITOR_WAYLANDPOINTER_H

#include <LiriAuroraCompositor/WaylandCompositorExtension>

struct wl_resource;

namespace Aurora {

namespace Compositor {

class WaylandPointer;
class WaylandPointerPrivate;
class WaylandSeat;
class WaylandView;
class WaylandOutput;
class WaylandClient;

class LIRIAURORACOMPOSITOR_EXPORT WaylandPointer : public WaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandPointer)
    Q_PROPERTY(bool isButtonPressed READ isButtonPressed NOTIFY buttonPressedChanged)
public:
    WaylandPointer(WaylandSeat *seat, QObject *parent = nullptr);
    ~WaylandPointer();

    WaylandSeat *seat() const;
    WaylandCompositor *compositor() const;

    WaylandOutput *output() const;
    void setOutput(WaylandOutput *output);

    virtual uint sendMousePressEvent(Qt::MouseButton button);
    virtual uint sendMouseReleaseEvent(Qt::MouseButton button);
    virtual void sendMouseMoveEvent(WaylandView *view, const QPointF &localPos, const QPointF &outputSpacePos);
    virtual void sendMouseWheelEvent(Qt::Orientation orientation, int delta);

    WaylandView *mouseFocus() const;
    QPointF currentLocalPosition() const;
    QPointF currentSpacePosition() const;

    bool isButtonPressed() const;

    virtual void addClient(WaylandClient *client, uint32_t id, uint32_t version);

    wl_resource *focusResource() const;

    static uint32_t toWaylandButton(Qt::MouseButton button);
    uint sendButton(struct wl_resource *resource, uint32_t time, Qt::MouseButton button, uint32_t state);
Q_SIGNALS:
    void outputChanged();
    void buttonPressedChanged();

private:
    QScopedPointer<WaylandPointerPrivate> const d_ptr;

    void enteredSurfaceDestroyed(void *data);
    void pointerFocusChanged(WaylandView *newFocus, WaylandView *oldFocus);
};

} // namespace Compositor

} // namespace Aurora

#endif  /*QWAYLANDPOINTER_H*/
