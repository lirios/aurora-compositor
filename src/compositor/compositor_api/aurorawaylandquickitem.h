// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDSURFACEITEM_H
#define AURORA_COMPOSITOR_WAYLANDSURFACEITEM_H

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>

#include <QtQuick/QQuickItem>
#include <QtQuick/qsgtexture.h>

#include <QtQuick/qsgtextureprovider.h>

#include <LiriAuroraCompositor/aurorawaylandview.h>
#include <LiriAuroraCompositor/aurorawaylandquicksurface.h>

namespace Aurora {

namespace Compositor {

class WaylandSeat;
class WaylandQuickItemPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickItem : public QQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandQuickItem)
    Q_PROPERTY(Aurora::Compositor::WaylandCompositor *compositor READ compositor NOTIFY compositorChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandSurface *surface READ surface WRITE setSurface NOTIFY surfaceChanged)
    Q_PROPERTY(bool paintEnabled READ isPaintEnabled WRITE setPaintEnabled NOTIFY paintEnabledChanged)
    Q_PROPERTY(bool touchEventsEnabled READ touchEventsEnabled WRITE setTouchEventsEnabled NOTIFY touchEventsEnabledChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandSurface::Origin origin READ origin NOTIFY originChanged)
    Q_PROPERTY(bool inputEventsEnabled READ inputEventsEnabled WRITE setInputEventsEnabled NOTIFY inputEventsEnabledChanged)
    Q_PROPERTY(bool focusOnClick READ focusOnClick WRITE setFocusOnClick NOTIFY focusOnClickChanged)
    Q_PROPERTY(QObject *subsurfaceHandler READ subsurfaceHandler WRITE setSubsurfaceHandler NOTIFY subsurfaceHandlerChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandOutput *output READ output WRITE setOutput NOTIFY outputChanged)
    Q_PROPERTY(bool bufferLocked READ isBufferLocked WRITE setBufferLocked NOTIFY bufferLockedChanged)
    Q_PROPERTY(bool allowDiscardFrontBuffer READ allowDiscardFrontBuffer WRITE setAllowDiscardFrontBuffer NOTIFY allowDiscardFrontBufferChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    Q_MOC_INCLUDE("aurorawaylandcompositor.h")
    Q_MOC_INCLUDE("aurorawaylandseat.h")
    Q_MOC_INCLUDE("aurorawaylanddrag.h")
    QML_NAMED_ELEMENT(WaylandQuickItem)
    QML_ADDED_IN_VERSION(1, 0)
#endif
public:
    WaylandQuickItem(QQuickItem *parent = nullptr);
    ~WaylandQuickItem() override;

    WaylandCompositor *compositor() const;
    WaylandView *view() const;

    WaylandSurface *surface() const;
    void setSurface(WaylandSurface *surface);

    WaylandSurface::Origin origin() const;

    bool isTextureProvider() const override;
    QSGTextureProvider *textureProvider() const override;

    bool isPaintEnabled() const;
    bool touchEventsEnabled() const;

    void setTouchEventsEnabled(bool enabled);

    bool inputEventsEnabled() const;
    void setInputEventsEnabled(bool enabled);

    bool focusOnClick() const;
    void setFocusOnClick(bool focus);

    bool inputRegionContains(const QPointF &localPosition) const;
    Q_INVOKABLE QPointF mapToSurface(const QPointF &point) const;
    Q_INVOKABLE QPointF mapFromSurface(const QPointF &point) const;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    Q_INVOKABLE QVariant inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const;
#endif
#endif

    QObject *subsurfaceHandler() const;
    void setSubsurfaceHandler(QObject*);

    WaylandOutput *output() const;
    void setOutput(WaylandOutput *output);

    bool isBufferLocked() const;
    void setBufferLocked(bool locked);

    bool allowDiscardFrontBuffer() const;
    void setAllowDiscardFrontBuffer(bool discard);

    Q_INVOKABLE void setPrimary();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override;
#endif

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void touchEvent(QTouchEvent *event) override;
    void touchUngrabEvent() override;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
    void inputMethodEvent(QInputMethodEvent *event) override;
#endif
#endif

    virtual void surfaceChangedEvent(Aurora::Compositor::WaylandSurface *newSurface, Aurora::Compositor::WaylandSurface *oldSurface);
public Q_SLOTS:
    virtual void takeFocus(Aurora::Compositor::WaylandSeat *device = nullptr);
    void setPaintEnabled(bool paintEnabled);
    void raise();
    void lower();
    void sendMouseMoveEvent(const QPointF &position, Aurora::Compositor::WaylandSeat *seat = nullptr);

private Q_SLOTS:
    void surfaceMappedChanged();
    void handleSurfaceChanged();
    void parentChanged(Aurora::Compositor::WaylandSurface *newParent, Aurora::Compositor::WaylandSurface *oldParent);
    void updateSize();
    void updateBuffer(bool hasBuffer);
    void updateWindow();
    void updateOutput();
    void beforeSync();
    void handleSubsurfaceAdded(Aurora::Compositor::WaylandSurface *childSurface);
    void handleSubsurfacePosition(const QPoint &pos);
    void handlePlaceAbove(Aurora::Compositor::WaylandSurface *referenceSurface);
    void handlePlaceBelow(Aurora::Compositor::WaylandSurface *referenceSurface);
    void handleDragStarted(Aurora::Compositor::WaylandDrag *drag);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
    void updateInputMethod(Qt::InputMethodQueries queries);
#endif
#endif
    void updateFocus();

Q_SIGNALS:
    void surfaceChanged();
    void compositorChanged();
    void paintEnabledChanged();
    void touchEventsEnabledChanged();
    void originChanged();
    void surfaceDestroyed();
    void inputEventsEnabledChanged();
    void focusOnClickChanged();
    void mouseMove(const QPointF &windowPosition);
    void mouseRelease();
    void subsurfaceHandlerChanged();
    void outputChanged();
    void bufferLockedChanged();
    void allowDiscardFrontBufferChanged();
protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

private:
    QScopedPointer<WaylandQuickItemPrivate> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

#endif
