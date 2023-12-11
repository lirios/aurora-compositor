// Copyright (C) 2017-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/aurorawaylandcompositor.h>
#include <LiriAuroraCompositor/aurorawaylandcompositorextension.h>
#include <LiriAuroraCompositor/aurorawaylandclient.h>
#include <LiriAuroraCompositor/aurorawaylanddrag.h>

#include <QtCore/QScopedPointer>
#include <QtGui/QImage>
#include <QtGui/QWindow>
#include <QtCore/QVariantMap>

struct wl_client;
struct wl_resource;

class QTouchEvent;

namespace Aurora {

namespace Compositor {

class WaylandSurfacePrivate;
class WaylandBufferRef;
class WaylandView;
class WaylandInputMethodControl;

class WaylandSurfaceRole
{
public:
    WaylandSurfaceRole(const QByteArray &n) : m_name(n) {}

    const QByteArray name() { return m_name; }

private:
    QByteArray m_name;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandSurface : public WaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandSurface)
    Q_PROPERTY(Aurora::Compositor::WaylandClient *client READ client CONSTANT)
    Q_PROPERTY(QRectF sourceGeometry READ sourceGeometry NOTIFY sourceGeometryChanged)
    Q_PROPERTY(QSize destinationSize READ destinationSize NOTIFY destinationSizeChanged)
    Q_PROPERTY(QSize bufferSize READ bufferSize NOTIFY bufferSizeChanged)
    Q_PROPERTY(int bufferScale READ bufferScale NOTIFY bufferScaleChanged)
    Q_PROPERTY(Qt::ScreenOrientation contentOrientation READ contentOrientation NOTIFY contentOrientationChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandSurface::Origin origin READ origin NOTIFY originChanged)
    Q_PROPERTY(bool hasContent READ hasContent NOTIFY hasContentChanged)
    Q_PROPERTY(bool cursorSurface READ isCursorSurface WRITE markAsCursorSurface NOTIFY cursorSurfaceChanged)
    Q_PROPERTY(bool inhibitsIdle READ inhibitsIdle NOTIFY inhibitsIdleChanged)
    Q_PROPERTY(bool isOpaque READ isOpaque NOTIFY isOpaqueChanged)
    Q_MOC_INCLUDE("aurorawaylanddrag.h")
    Q_MOC_INCLUDE("aurorawaylandcompositor.h")

    QML_NAMED_ELEMENT(WaylandSurfaceBase)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("Cannot create instance of WaylandSurfaceBase, use WaylandSurface instead")
public:
    enum Origin {
        OriginTopLeft,
        OriginBottomLeft
    };
    Q_ENUM(Origin)

    WaylandSurface();
    WaylandSurface(WaylandCompositor *compositor, WaylandClient *client, uint id, int version);
    ~WaylandSurface() override;

    Q_INVOKABLE void initialize(Aurora::Compositor::WaylandCompositor *compositor, Aurora::Compositor::WaylandClient *client, uint id, int version);
    bool isInitialized() const;

    WaylandClient *client() const;
    ::wl_client *waylandClient() const;

    bool setRole(WaylandSurfaceRole *role, wl_resource *errorResource, uint32_t errorCode);
    WaylandSurfaceRole *role() const;

    bool hasContent() const;

    QRectF sourceGeometry() const;
    QSize destinationSize() const;
    QSize bufferSize() const;
    int bufferScale() const;

    Qt::ScreenOrientation contentOrientation() const;

    Origin origin() const;

    WaylandCompositor *compositor() const;

    bool inputRegionContains(const QPoint &p) const;
    bool inputRegionContains(const QPointF &position) const;

    Q_INVOKABLE void destroy();
    Q_INVOKABLE bool isDestroyed() const;

    Q_INVOKABLE void frameStarted();
    Q_INVOKABLE void sendFrameCallbacks();

    WaylandView *primaryView() const;
    void setPrimaryView(WaylandView *view);

    QList<WaylandView *> views() const;

    static WaylandSurface *fromResource(::wl_resource *resource);
    struct wl_resource *resource() const;

    void markAsCursorSurface(bool cursorSurface);
    bool isCursorSurface() const;

    bool inhibitsIdle() const;
    bool isOpaque() const;

#if QT_CONFIG(im)
    WaylandInputMethodControl *inputMethodControl() const;
#endif

public Q_SLOTS:
#if QT_CONFIG(clipboard)
    void updateSelection();
#endif

protected:
    WaylandSurface(WaylandSurfacePrivate &dptr);

Q_SIGNALS:
    void hasContentChanged();
    void damaged(const QRegion &rect);
    void parentChanged(WaylandSurface *newParent, WaylandSurface *oldParent);
    void childAdded(WaylandSurface *child);
    void sourceGeometryChanged();
    void destinationSizeChanged();
    void bufferSizeChanged();
    void bufferScaleChanged();
    void offsetForNextFrame(const QPoint &offset);
    void contentOrientationChanged();
    void surfaceDestroyed();
    void originChanged();
    void subsurfacePositionChanged(const QPoint &position);
    void subsurfacePlaceAbove(WaylandSurface *sibling);
    void subsurfacePlaceBelow(WaylandSurface *sibling);
    void dragStarted(WaylandDrag *drag);
    void cursorSurfaceChanged();
    void inhibitsIdleChanged();
    void isOpaqueChanged();

    void configure(bool hasBuffer);
    void redraw();
};

} // namespace Compositor

} // namespace Aurora

