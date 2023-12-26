// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QtCore/qnamespace.h>
#include <QtCore/QPoint>
#include <QtCore/QString>

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/auroraqmlinclude.h>
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

#if QT_CONFIG(draganddrop)
    Q_PROPERTY(Aurora::Compositor::WaylandDrag *drag READ drag CONSTANT)
    Q_MOC_INCLUDE("aurorawaylanddrag.h")
#endif
    Q_PROPERTY(Aurora::Compositor::WaylandKeymap *keymap READ keymap CONSTANT)
    Q_MOC_INCLUDE("aurorawaylandkeymap.h")
    Q_MOC_INCLUDE("aurorawaylandview.h")

    QML_NAMED_ELEMENT(WaylandSeat)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("")
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
    Q_INVOKABLE void sendUnicodeKeyEvent(uint unicode, bool pressed);

    uint sendTouchPointEvent(WaylandSurface *surface, int id, const QPointF &point, Qt::TouchPointState state);
    Q_INVOKABLE uint sendTouchPointPressed(Aurora::Compositor::WaylandSurface *surface, int id, const QPointF &position);
    Q_INVOKABLE uint sendTouchPointReleased(Aurora::Compositor::WaylandSurface *surface, int id, const QPointF &position);
    Q_INVOKABLE uint sendTouchPointMoved(Aurora::Compositor::WaylandSurface *surface, int id, const QPointF &position);
    Q_INVOKABLE void sendTouchFrameEvent(Aurora::Compositor::WaylandClient *client);
    Q_INVOKABLE void sendTouchCancelEvent(Aurora::Compositor::WaylandClient *client);

    void sendFullTouchEvent(WaylandSurface *surface, QTouchEvent *event);

    bool isInputAllowed(WaylandSurface *surface) const;

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

#if QT_CONFIG(draganddrop)
    WaylandDrag *drag() const;
#endif

    WaylandSeat::CapabilityFlags capabilities() const;

    virtual bool isOwner(QEvent *inputEvent) const;

    static WaylandSeat *fromSeatResource(struct ::wl_resource *resource);

Q_SIGNALS:
    void pointerChanged(Aurora::Compositor::WaylandPointer *pointer);
    void keyboardChanged(Aurora::Compositor::WaylandKeyboard *keyboard);
    void touchChanged(Aurora::Compositor::WaylandTouch *touch);
    void mouseFocusChanged(Aurora::Compositor::WaylandView *newFocus, Aurora::Compositor::WaylandView *oldFocus);
    void keyboardFocusChanged(Aurora::Compositor::WaylandSurface *newFocus, Aurora::Compositor::WaylandSurface *oldFocus);
    void cursorSurfaceRequested(Aurora::Compositor::WaylandSurface *surface, int hotspotX, int hotspotY, Aurora::Compositor::WaylandClient *client);

private:
    void handleMouseFocusDestroyed();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WaylandSeat::CapabilityFlags)

} // namespace Compositor

} // namespace Aurora

