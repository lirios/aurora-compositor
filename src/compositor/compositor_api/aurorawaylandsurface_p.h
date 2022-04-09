/****************************************************************************
**
** Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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

#ifndef AURORA_COMPOSITOR_WAYLANDSURFACE_P_H
#define AURORA_COMPOSITOR_WAYLANDSURFACE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Aurora API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <private/qobject_p.h>

#include <private/aurorawlclientbuffer_p.h>
#include <LiriAuroraCompositor/aurorawaylandsurface.h>
#include <LiriAuroraCompositor/aurorawaylandbufferref.h>

#include <LiriAuroraCompositor/private/aurorawlregion_p.h>

#include <QtCore/QList>
#include <QtCore/QRect>
#include <QtGui/QRegion>
#include <QtGui/QImage>
#include <QtGui/QWindow>

#include <QtCore/QTextStream>
#include <QtCore/QMetaType>

#include <wayland-util.h>

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>
#include <LiriAuroraCompositor/private/aurorawaylandviewporter_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandidleinhibitv1_p.h>

namespace Aurora {

namespace Compositor {

class WaylandCompositor;
class WaylandSurface;
class WaylandView;
class WaylandInputMethodControl;

namespace QtWayland {
class FrameCallback;
}

class LIRIAURORACOMPOSITOR_EXPORT WaylandSurfacePrivate : public QObjectPrivate, public PrivateServer::wl_surface
{
public:
    static WaylandSurfacePrivate *get(WaylandSurface *surface);

    WaylandSurfacePrivate();
    ~WaylandSurfacePrivate() override;

    void ref();
    void deref();

    void refView(WaylandView *view);
    void derefView(WaylandView *view);

    using PrivateServer::wl_surface::resource;

    void removeFrameCallback(QtWayland::FrameCallback *callback);

    void notifyViewsAboutDestruction();

#ifndef QT_NO_DEBUG
    static void addUninitializedSurface(WaylandSurfacePrivate *surface);
    static void removeUninitializedSurface(WaylandSurfacePrivate *surface);
    static bool hasUninitializedSurface();
#endif

    void initSubsurface(WaylandSurface *parent, struct ::wl_client *client, int id, int version);
    bool isSubsurface() const { return subsurface; }
    WaylandSurfacePrivate *parentSurface() const { return subsurface ? subsurface->parentSurface : nullptr; }

protected:
    void surface_destroy_resource(Resource *resource) override;

    void surface_destroy(Resource *resource) override;
    void surface_attach(Resource *resource,
                        struct wl_resource *buffer, int x, int y) override;
    void surface_damage(Resource *resource,
                        int32_t x, int32_t y, int32_t width, int32_t height) override;
    void surface_damage_buffer(Resource *resource, int32_t x, int32_t y, int32_t width, int32_t height) override;
    void surface_frame(Resource *resource,
                       uint32_t callback) override;
    void surface_set_opaque_region(Resource *resource,
                                   struct wl_resource *region) override;
    void surface_set_input_region(Resource *resource,
                                  struct wl_resource *region) override;
    void surface_commit(Resource *resource) override;
    void surface_set_buffer_transform(Resource *resource, int32_t transform) override;
    void surface_set_buffer_scale(Resource *resource, int32_t bufferScale) override;

    QtWayland::ClientBuffer *getBuffer(struct ::wl_resource *buffer);

public: //member variables
    WaylandCompositor *compositor = nullptr;
    int refCount = 1;
    WaylandClient *client = nullptr;
    QList<WaylandView *> views;
    QRegion damage;
    WaylandBufferRef bufferRef;
    WaylandSurfaceRole *role = nullptr;
    WaylandViewporterPrivate::Viewport *viewport = nullptr;

    struct {
        WaylandBufferRef buffer;
        QRegion damage;
        bool damageInBufferCoordinates = false;
        QPoint offset;
        bool newlyAttached = false;
        QRegion inputRegion;
        int bufferScale = 1;
        QRectF sourceGeometry;
        QSize destinationSize;
        QRegion opaqueRegion;
    } pending;

    QPoint lastLocalMousePos;
    QPoint lastGlobalMousePos;

    QList<QtWayland::FrameCallback *> pendingFrameCallbacks;
    QList<QtWayland::FrameCallback *> frameCallbacks;

    QList<QPointer<WaylandSurface>> subsurfaceChildren;

    QList<WaylandIdleInhibitManagerV1Private::Inhibitor *> idleInhibitors;

    QRegion inputRegion;
    QRegion opaqueRegion;

    QRectF sourceGeometry;
    QSize destinationSize;
    QSize bufferSize;
    int bufferScale = 1;
    bool isCursorSurface = false;
    bool destroyed = false;
    bool hasContent = false;
    bool isInitialized = false;
    bool isOpaque = false;
    Qt::ScreenOrientation contentOrientation = Qt::PrimaryOrientation;
    QWindow::Visibility visibility;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
    WaylandInputMethodControl *inputMethodControl = nullptr;
#endif
#endif

    class Subsurface : public PrivateServer::wl_subsurface
    {
    public:
        Subsurface(WaylandSurfacePrivate *s) : surface(s) {}
        WaylandSurfacePrivate *surfaceFromResource();

    protected:
        void subsurface_set_position(wl_subsurface::Resource *resource, int32_t x, int32_t y) override;
        void subsurface_place_above(wl_subsurface::Resource *resource, struct wl_resource *sibling) override;
        void subsurface_place_below(wl_subsurface::Resource *resource, struct wl_resource *sibling) override;
        void subsurface_set_sync(wl_subsurface::Resource *resource) override;
        void subsurface_set_desync(wl_subsurface::Resource *resource) override;

    private:
        friend class WaylandSurfacePrivate;
        WaylandSurfacePrivate *surface = nullptr;
        WaylandSurfacePrivate *parentSurface = nullptr;
        QPoint position;
    };

    Subsurface *subsurface = nullptr;

#ifndef QT_NO_DEBUG
    static QList<WaylandSurfacePrivate *> uninitializedSurfaces;
#endif
    Q_DECLARE_PUBLIC(WaylandSurface)
    Q_DISABLE_COPY(WaylandSurfacePrivate)
};

} // namespace Compositor

} // namespace Aurora

#endif
