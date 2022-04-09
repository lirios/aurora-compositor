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

#ifndef AURORA_COMPOSITOR_WAYLANDSURFACEITEM_H
#define AURORA_COMPOSITOR_WAYLANDSURFACEITEM_H

#include <LiriAuroraCompositor/qtwaylandcompositorglobal.h>

#include <QtQuick/QQuickItem>
#include <QtQuick/qsgtexture.h>

#include <QtQuick/qsgtextureprovider.h>

#include <LiriAuroraCompositor/aurorawaylandview.h>
#include <LiriAuroraCompositor/aurorawaylandquicksurface.h>

Q_DECLARE_METATYPE(WaylandQuickSurface*)

QT_REQUIRE_CONFIG(wayland_compositor_quick);

namespace Aurora {

namespace Compositor {

class WaylandSeat;
class WaylandQuickItemPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandQuickItem : public QQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandQuickItem)
    Q_PROPERTY(WaylandCompositor *compositor READ compositor NOTIFY compositorChanged)
    Q_PROPERTY(WaylandSurface *surface READ surface WRITE setSurface NOTIFY surfaceChanged)
    Q_PROPERTY(bool paintEnabled READ isPaintEnabled WRITE setPaintEnabled NOTIFY paintEnabledChanged)
    Q_PROPERTY(bool touchEventsEnabled READ touchEventsEnabled WRITE setTouchEventsEnabled NOTIFY touchEventsEnabledChanged)
    Q_PROPERTY(WaylandSurface::Origin origin READ origin NOTIFY originChanged)
    Q_PROPERTY(bool inputEventsEnabled READ inputEventsEnabled WRITE setInputEventsEnabled NOTIFY inputEventsEnabledChanged)
    Q_PROPERTY(bool focusOnClick READ focusOnClick WRITE setFocusOnClick NOTIFY focusOnClickChanged)
    Q_PROPERTY(QObject *subsurfaceHandler READ subsurfaceHandler WRITE setSubsurfaceHandler NOTIFY subsurfaceHandlerChanged)
    Q_PROPERTY(WaylandOutput *output READ output WRITE setOutput NOTIFY outputChanged)
    Q_PROPERTY(bool bufferLocked READ isBufferLocked WRITE setBufferLocked NOTIFY bufferLockedChanged)
    Q_PROPERTY(bool allowDiscardFrontBuffer READ allowDiscardFrontBuffer WRITE setAllowDiscardFrontBuffer NOTIFY allowDiscardFrontBufferChanged)
    Q_MOC_INCLUDE("aurorawaylandcompositor.h")
    Q_MOC_INCLUDE("aurorawaylandseat.h")
    Q_MOC_INCLUDE("aurorawaylanddrag.h")
    QML_NAMED_ELEMENT(WaylandQuickItem)
    QML_ADDED_IN_VERSION(1, 0)
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
    Q_REVISION(1, 13) Q_INVOKABLE QPointF mapFromSurface(const QPointF &point) const;

#if QT_CONFIG(im)
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    Q_INVOKABLE QVariant inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const;
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

#if QT_CONFIG(im)
    void inputMethodEvent(QInputMethodEvent *event) override;
#endif

    virtual void surfaceChangedEvent(WaylandSurface *newSurface, WaylandSurface *oldSurface);
public Q_SLOTS:
    virtual void takeFocus(WaylandSeat *device = nullptr);
    void setPaintEnabled(bool paintEnabled);
    void raise();
    void lower();
    void sendMouseMoveEvent(const QPointF &position, WaylandSeat *seat = nullptr);

private Q_SLOTS:
    void surfaceMappedChanged();
    void handleSurfaceChanged();
    void parentChanged(WaylandSurface *newParent, WaylandSurface *oldParent);
    void updateSize();
    void updateBuffer(bool hasBuffer);
    void updateWindow();
    void updateOutput();
    void beforeSync();
    void handleSubsurfaceAdded(WaylandSurface *childSurface);
    void handleSubsurfacePosition(const QPoint &pos);
    void handlePlaceAbove(WaylandSurface *referenceSurface);
    void handlePlaceBelow(WaylandSurface *referenceSurface);
#if QT_CONFIG(draganddrop)
    void handleDragStarted(WaylandDrag *drag);
#endif
#if QT_CONFIG(im)
    void updateInputMethod(Qt::InputMethodQueries queries);
#endif

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

    WaylandQuickItem(WaylandQuickItemPrivate &dd, QQuickItem *parent = nullptr);
};

} // namespace Compositor

} // namespace Aurora

#endif
