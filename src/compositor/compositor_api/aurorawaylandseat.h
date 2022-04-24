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

#ifndef AURORA_COMPOSITOR_WAYLANDSEAT_H
#define AURORA_COMPOSITOR_WAYLANDSEAT_H

#include <QtCore/qnamespace.h>
#include <QtCore/QPoint>
#include <QtCore/QString>

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/aurorawaylandqmlinclude.h>
#include <LiriAuroraCompositor/aurorawaylandcompositorextension.h>
#include <LiriAuroraCompositor/aurorawaylandkeyboard.h>
#include <LiriAuroraCompositor/aurorawaylandview.h>

class QKeyEvent;
class QTouchEvent;
class QInputEvent;

namespace Aurora {

namespace Compositor {

class WaylandCompositor;
class WaylandSurface;
class WaylandSeatPrivate;
class WaylandDrag;
class WaylandKeyboard;
class WaylandPointer;
class WaylandTouch;

class LIRIAURORACOMPOSITOR_EXPORT WaylandSeat : public WaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandSeat)

    Q_PROPERTY(Aurora::Compositor::WaylandDrag *drag READ drag CONSTANT)
    Q_PROPERTY(Aurora::Compositor::WaylandKeymap *keymap READ keymap CONSTANT)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    Q_MOC_INCLUDE("aurorawaylanddrag.h")
    Q_MOC_INCLUDE("aurorawaylandkeymap.h")
    Q_MOC_INCLUDE("aurorawaylandview.h")
    QML_NAMED_ELEMENT(WaylandSeat)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("")
#endif
public:
    enum CapabilityFlag {
        // The order should match the enum WL_SEAT_CAPABILITY_*
        Pointer = 0x01,
        Keyboard = 0x02,
        Touch = 0x04,

        DefaultCapabilities = Pointer | Keyboard | Touch
    };
    Q_DECLARE_FLAGS(CapabilityFlags, CapabilityFlag)
    Q_ENUM(CapabilityFlags)

    WaylandSeat(WaylandCompositor *compositor, CapabilityFlags capabilityFlags = DefaultCapabilities);
    ~WaylandSeat() override;
    virtual void initialize();
    bool isInitialized() const;

    void sendMousePressEvent(Qt::MouseButton button);
    void sendMouseReleaseEvent(Qt::MouseButton button);
    void sendMouseMoveEvent(WaylandView *surface , const QPointF &localPos, const QPointF &outputSpacePos = QPointF());
    void sendMouseWheelEvent(Qt::Orientation orientation, int delta);

    void sendKeyPressEvent(uint code);
    void sendKeyReleaseEvent(uint code);

    void sendFullKeyEvent(QKeyEvent *event);
    Q_INVOKABLE void sendKeyEvent(int qtKey, bool pressed);

    uint sendTouchPointEvent(WaylandSurface *surface, int id, const QPointF &point, Qt::TouchPointState state);
    Q_INVOKABLE uint sendTouchPointPressed(Aurora::Compositor::WaylandSurface *surface, int id, const QPointF &position);
    Q_INVOKABLE uint sendTouchPointReleased(Aurora::Compositor::WaylandSurface *surface, int id, const QPointF &position);
    Q_INVOKABLE uint sendTouchPointMoved(Aurora::Compositor::WaylandSurface *surface, int id, const QPointF &position);
    Q_INVOKABLE void sendTouchFrameEvent(Aurora::Compositor::WaylandClient *client);
    Q_INVOKABLE void sendTouchCancelEvent(Aurora::Compositor::WaylandClient *client);

    void sendFullTouchEvent(WaylandSurface *surface, QTouchEvent *event);

    WaylandPointer *pointer() const;
    //Normally set by the mouse device,
    //But can be set manually for use with touch or can reset unset the current mouse focus;
    WaylandView *mouseFocus() const;
    void setMouseFocus(WaylandView *view);

    WaylandKeyboard *keyboard() const;
    WaylandSurface *keyboardFocus() const;
    bool setKeyboardFocus(WaylandSurface *surface);
    WaylandKeymap *keymap();

    WaylandTouch *touch() const;

    WaylandCompositor *compositor() const;

    WaylandDrag *drag() const;

    WaylandSeat::CapabilityFlags capabilities() const;

    virtual bool isOwner(QInputEvent *inputEvent) const;

    static WaylandSeat *fromSeatResource(struct ::wl_resource *resource);

Q_SIGNALS:
    void mouseFocusChanged(Aurora::Compositor::WaylandView *newFocus, Aurora::Compositor::WaylandView *oldFocus);
    void keyboardFocusChanged(Aurora::Compositor::WaylandSurface *newFocus, Aurora::Compositor::WaylandSurface *oldFocus);
    void cursorSurfaceRequested(Aurora::Compositor::WaylandSurface *surface, int hotspotX, int hotspotY, Aurora::Compositor::WaylandClient *client);

private:
    QScopedPointer<WaylandSeatPrivate> const d_ptr;
    void handleMouseFocusDestroyed();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WaylandSeat::CapabilityFlags)

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDSEAT_H
