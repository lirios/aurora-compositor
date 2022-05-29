/****************************************************************************
**
** Copyright (C) 2017-2015 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "aurorawaylandsurface.h"
#include "aurorawaylandsurface_p.h"

#include "wayland_wrapper/aurorawlbuffermanager_p.h"
#include "wayland_wrapper/aurorawlregion_p.h"
#if LIRI_FEATURE_aurora_datadevice
#include "wayland_wrapper/aurorawldatadevice_p.h"
#include "wayland_wrapper/aurorawldatadevicemanager_p.h"
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
#include "aurorawaylandinputmethodcontrol_p.h"
#endif
#endif

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandClient>
#include <LiriAuroraCompositor/WaylandView>
#include <LiriAuroraCompositor/WaylandBufferRef>

#include <LiriAuroraCompositor/private/aurorawaylandcompositor_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandview_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandseat_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandutils_p.h>

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include <QtCore/QDebug>
#include <QtCore/QtMath>

namespace Aurora {

namespace Compositor {

namespace Internal {
class FrameCallback {
public:
    FrameCallback(WaylandSurface *surf, wl_resource *res)
        : surface(surf)
        , resource(res)
    {
        wl_resource_set_implementation(res, nullptr, this, destroyCallback);
    }
    ~FrameCallback()
    {
    }
    void destroy()
    {
        if (resource)
            wl_resource_destroy(resource);
        else
            delete this;
    }
    void send(uint time)
    {
        wl_callback_send_done(resource, time);
        wl_resource_destroy(resource);
    }
    static void destroyCallback(wl_resource *res)
    {
        FrameCallback *_this = static_cast<FrameCallback *>(wl_resource_get_user_data(res));
        if (_this->surface)
            WaylandSurfacePrivate::get(_this->surface)->removeFrameCallback(_this);
        delete _this;
    }
    WaylandSurface *surface = nullptr;
    wl_resource *resource = nullptr;
    bool canSend = false;
};
}
static QRegion infiniteRegion() {
    return QRegion(QRect(QPoint(std::numeric_limits<int>::min(), std::numeric_limits<int>::min()),
                         QPoint(std::numeric_limits<int>::max(), std::numeric_limits<int>::max())));
}

#ifndef QT_NO_DEBUG
QList<WaylandSurfacePrivate *> WaylandSurfacePrivate::uninitializedSurfaces;
#endif

WaylandSurfacePrivate::WaylandSurfacePrivate(WaylandSurface *self)
    : q_ptr(self)
    , inputRegion(infiniteRegion())
{
    pending.buffer = WaylandBufferRef();
    pending.newlyAttached = false;
    pending.inputRegion = infiniteRegion();
    pending.bufferScale = 1;
#ifndef QT_NO_DEBUG
    addUninitializedSurface(this);
#endif
}

WaylandSurfacePrivate::~WaylandSurfacePrivate()
{
    for (int i = 0; i < views.size(); i++) {
        WaylandViewPrivate::get(views.at(i))->markSurfaceAsDestroyed(q_func());
    }
    views.clear();

    bufferRef = WaylandBufferRef();

    for (Internal::FrameCallback *c : qAsConst(pendingFrameCallbacks))
        c->destroy();
    for (Internal::FrameCallback *c : qAsConst(frameCallbacks))
        c->destroy();
}

void WaylandSurfacePrivate::removeFrameCallback(Internal::FrameCallback *callback)
{
    pendingFrameCallbacks.removeOne(callback);
    frameCallbacks.removeOne(callback);
}

void WaylandSurfacePrivate::notifyViewsAboutDestruction()
{
    Q_Q(WaylandSurface);
    const auto viewsCopy = views; // Views will be removed from the list when marked as destroyed
    for (WaylandView *view : viewsCopy) {
        WaylandViewPrivate::get(view)->markSurfaceAsDestroyed(q);
    }
    if (hasContent) {
        hasContent = false;
        emit q->hasContentChanged();
    }
}

#ifndef QT_NO_DEBUG
void WaylandSurfacePrivate::addUninitializedSurface(WaylandSurfacePrivate *surface)
{
    Q_ASSERT(!surface->isInitialized);
    Q_ASSERT(!uninitializedSurfaces.contains(surface));
    uninitializedSurfaces.append(surface);
}

void WaylandSurfacePrivate::removeUninitializedSurface(WaylandSurfacePrivate *surface)
{
    Q_ASSERT(surface->isInitialized);
    bool removed = uninitializedSurfaces.removeOne(surface);
    Q_ASSERT(removed);
}

bool WaylandSurfacePrivate::hasUninitializedSurface()
{
    return uninitializedSurfaces.size();
}
#endif

void WaylandSurfacePrivate::surface_destroy_resource(Resource *)
{
    Q_Q(WaylandSurface);
    notifyViewsAboutDestruction();

    destroyed = true;
    emit q->surfaceDestroyed();
    q->destroy();
}

void WaylandSurfacePrivate::surface_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void WaylandSurfacePrivate::surface_attach(Resource *, struct wl_resource *buffer, int x, int y)
{
    pending.buffer = WaylandBufferRef(getBuffer(buffer));
    pending.offset = QPoint(x, y);
    pending.newlyAttached = true;
}

/*
    Note: The Wayland protocol specifies that buffer scale and damage can be interleaved, so
    we cannot scale the damage region until commit. We assume that clients will either use
    surface_damage or surface_damage_buffer within one frame for one surface.
*/

void WaylandSurfacePrivate::surface_damage(Resource *, int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (Q_UNLIKELY(pending.damageInBufferCoordinates && !pending.damage.isNull()))
        qCWarning(gLcAuroraCompositor) << "Unsupported: Client is using both wl_surface.damage_buffer and wl_surface.damage.";
    pending.damage = pending.damage.united(QRect(x, y, width, height));
    pending.damageInBufferCoordinates = false;
}

void WaylandSurfacePrivate::surface_damage_buffer(Resource *, int32_t x, int32_t y, int32_t width, int32_t height)
{
    if (Q_UNLIKELY(!pending.damageInBufferCoordinates && !pending.damage.isNull()))
        qCWarning(gLcAuroraCompositor) << "Unsupported: Client is using both wl_surface.damage_buffer and wl_surface.damage.";
    pending.damage = pending.damage.united(QRect(x, y, width, height));
    pending.damageInBufferCoordinates = true;
}

void WaylandSurfacePrivate::surface_frame(Resource *resource, uint32_t callback)
{
    Q_Q(WaylandSurface);
    struct wl_resource *frame_callback = wl_resource_create(resource->client(), &wl_callback_interface, wl_callback_interface.version, callback);
    pendingFrameCallbacks << new Internal::FrameCallback(q, frame_callback);
}

void WaylandSurfacePrivate::surface_set_opaque_region(Resource *, struct wl_resource *region)
{
    pending.opaqueRegion = region ? Internal::Region::fromResource(region)->region() : QRegion();
}

void WaylandSurfacePrivate::surface_set_input_region(Resource *, struct wl_resource *region)
{
    if (region) {
        pending.inputRegion = Internal::Region::fromResource(region)->region();
    } else {
        pending.inputRegion = infiniteRegion();
    }
}

void WaylandSurfacePrivate::surface_commit(Resource *)
{
    Q_Q(WaylandSurface);

    // Needed in order to know whether we want to emit signals later
    QSize oldBufferSize = bufferSize;
    QRectF oldSourceGeometry = sourceGeometry;
    QSize oldDestinationSize = destinationSize;
    bool oldHasContent = hasContent;
    int oldBufferScale = bufferScale;

    // Update all internal state
    if (pending.buffer.hasBuffer() || pending.newlyAttached)
        bufferRef = pending.buffer;
    bufferScale = pending.bufferScale;
    bufferSize = bufferRef.size();
    QSize surfaceSize = bufferSize / bufferScale;
    sourceGeometry = !pending.sourceGeometry.isValid() ? QRect(QPoint(), surfaceSize) : pending.sourceGeometry;
    destinationSize = pending.destinationSize.isEmpty() ? sourceGeometry.size().toSize() : pending.destinationSize;
    QRect destinationRect(QPoint(), destinationSize);
    if (!pending.damageInBufferCoordinates || pending.bufferScale == 1) {
        // pending.damage is already in surface coordinates
        damage = pending.damage.intersected(QRect(QPoint(), destinationSize));
    } else {
        // We must transform pending.damage from buffer coordinate system to surface coordinates
        // TODO(QTBUG-85461): Also support wp_viewport setting more complex transformations
        auto xform = [](const QRect &r, int scale) -> QRect {
            QRect res{
                QPoint{ r.x() / scale, r.y() / scale },
                QPoint{ (r.right() + scale - 1) / scale, (r.bottom() + scale - 1) / scale }
            };
            return res;
        };
        damage = {};
        for (const QRect &r : pending.damage) {
            damage |= xform(r, bufferScale).intersected(destinationRect);
        }
    }
    hasContent = bufferRef.hasContent();
    frameCallbacks << pendingFrameCallbacks;
    inputRegion = pending.inputRegion.intersected(destinationRect);
    opaqueRegion = pending.opaqueRegion.intersected(destinationRect);
    bool becameOpaque = opaqueRegion.boundingRect().contains(destinationRect);
    if (becameOpaque != isOpaque) {
        isOpaque = becameOpaque;
        emit q->isOpaqueChanged();
    }

    QPoint offsetForNextFrame = pending.offset;

    if (viewport)
        viewport->checkCommittedState();

    // Clear per-commit state
    pending.buffer = WaylandBufferRef();
    pending.offset = QPoint();
    pending.newlyAttached = false;
    pending.damage = QRegion();
    pending.damageInBufferCoordinates = false;
    pendingFrameCallbacks.clear();

    // Notify buffers and views
    if (auto *buffer = bufferRef.buffer())
        buffer->setCommitted(damage);
    for (auto *view : qAsConst(views))
        view->bufferCommitted(bufferRef, damage);

    // Now all double-buffered state has been applied so it's safe to emit general signals
    // i.e. we won't have inconsistensies such as mismatched surface size and buffer scale in
    // signal handlers.

    emit q->damaged(damage);

    if (oldBufferSize != bufferSize)
        emit q->bufferSizeChanged();

    if (oldBufferScale != bufferScale)
        emit q->bufferScaleChanged();

    if (oldDestinationSize != destinationSize)
        emit q->destinationSizeChanged();

    if (oldSourceGeometry != sourceGeometry)
        emit q->sourceGeometryChanged();

    if (oldHasContent != hasContent)
        emit q->hasContentChanged();

    if (!offsetForNextFrame.isNull())
        emit q->offsetForNextFrame(offsetForNextFrame);

    emit q->redraw();
}

void WaylandSurfacePrivate::surface_set_buffer_transform(Resource *resource, int32_t orientation)
{
    Q_UNUSED(resource);
    Q_Q(WaylandSurface);
    QScreen *screen = QGuiApplication::primaryScreen();
    bool isPortrait = screen->primaryOrientation() == Qt::PortraitOrientation;
    Qt::ScreenOrientation oldOrientation = contentOrientation;
    switch (orientation) {
        case WL_OUTPUT_TRANSFORM_90:
            contentOrientation = isPortrait ? Qt::InvertedLandscapeOrientation : Qt::PortraitOrientation;
            break;
        case WL_OUTPUT_TRANSFORM_180:
            contentOrientation = isPortrait ? Qt::InvertedPortraitOrientation : Qt::InvertedLandscapeOrientation;
            break;
        case WL_OUTPUT_TRANSFORM_270:
            contentOrientation = isPortrait ? Qt::LandscapeOrientation : Qt::InvertedPortraitOrientation;
            break;
        default:
            contentOrientation = Qt::PrimaryOrientation;
    }
    if (contentOrientation != oldOrientation)
        emit q->contentOrientationChanged();
}

void WaylandSurfacePrivate::surface_set_buffer_scale(PrivateServer::wl_surface::Resource *resource, int32_t scale)
{
    Q_UNUSED(resource);
    pending.bufferScale = scale;
}

Internal::ClientBuffer *WaylandSurfacePrivate::getBuffer(struct ::wl_resource *buffer)
{
    Internal::BufferManager *bufMan = WaylandCompositorPrivate::get(compositor)->bufferManager();
    return bufMan->getBuffer(buffer);
}

/*!
 * \class WaylandSurfaceRole
 * \inmodule AuroraCompositor
 * \since 5.8
 * \brief The WaylandSurfaceRole class represents the role of the surface in context of wl_surface.
 *
 * WaylandSurfaceRole is used to represent the role of a WaylandSurface. According to the protocol
 * specification, the role of a surface is permanent once set, and if the same surface is later
 * reused for a different role, this constitutes a protocol error. Setting the surface to the same
 * role multiple times is not an error.
 *
 * As an example, the WaylandXdgShell can assign either "popup" or "toplevel" roles to surfaces.
 * If \c get_toplevel is requested on a surface which has previously received a \c get_popup
 * request, then the compositor will issue a protocol error.
 *
 * Roles are compared by pointer value, so any two objects of WaylandSurfaceRole will be considered
 * different roles, regardless of what their \l{name()}{names} are. A typical way of assigning a
 * role is to have a static WaylandSurfaceRole object to represent it.
 *
 * \code
 * class MyShellSurfaceSubType
 * {
 *     static WaylandSurfaceRole s_role;
 *     // ...
 * };
 *
 * // ...
 *
 * surface->setRole(&MyShellSurfaceSubType::s_role, resource->handle, MY_ERROR_CODE);
 * \endcode
 */

/*!
 * \fn WaylandSurfaceRole::WaylandSurfaceRole(const QByteArray &name)
 *
 * Creates a WaylandSurfaceRole and assigns it \a name. The name is used in error messages
 * involving this WaylandSurfaceRole.
 */

/*!
 * \fn const QByteArray WaylandSurfaceRole::name()
 *
 * Returns the name of the WaylandSurfaceRole. The name is used in error messages involving this
 * WaylandSurfaceRole, for example if an attempt is made to change the role of a surface.
 */

/*!
 * \qmltype WaylandSurface
 * \instantiates WaylandSurface
 * \inqmlmodule Aurora.Compositor
 * \since 5.8
 * \brief Represents a rectangular area on an output device.
 *
 * This type encapsulates a rectangular area of pixels that is displayed on an output device. It
 * corresponds to the interface \c wl_surface in the Wayland protocol.
 */

/*!
 * \class WaylandSurface
 * \inmodule AuroraCompositor
 * \since 5.8
 * \brief The WaylandSurface class represents a rectangular area on an output device.
 *
 * This class encapsulates a rectangular area of pixels that is displayed on an output device. It
 * corresponds to the interface \c wl_surface in the Wayland protocol.
 */

/*!
 * Constructs a an uninitialized WaylandSurface.
 */
WaylandSurface::WaylandSurface()
    : WaylandObject()
    , d_ptr(new WaylandSurfacePrivate(this))
{
}

/*!
 * Constructs and initializes a WaylandSurface for the given \a compositor and \a client, and with the given \a id
 * and \a version.
 */
WaylandSurface::WaylandSurface(WaylandCompositor *compositor, WaylandClient *client, uint id, int version)
    : WaylandObject()
    , d_ptr(new WaylandSurfacePrivate(this))
{
    initialize(compositor, client, id, version);
}

/*!
 * Destroys the WaylandSurface.
 */
WaylandSurface::~WaylandSurface()
{
    Q_D(WaylandSurface);
    if (d->compositor)
        WaylandCompositorPrivate::get(d->compositor)->unregisterSurface(this);
    d->notifyViewsAboutDestruction();
}

/*!
 * \qmlmethod void AuroraCompositor::WaylandSurface::initialize(WaylandCompositor compositor, WaylandClient client, int id, int version)
 *
 * Initializes the WaylandSurface with the given \a compositor and \a client, and with the given \a id
 * and \a version.
 */

/*!
 * Initializes the WaylandSurface with the given \a compositor and \a client, and with the given \a id
 * and \a version.
 */
void WaylandSurface::initialize(WaylandCompositor *compositor, WaylandClient *client, uint id, int version)
{
    Q_D(WaylandSurface);
    d->compositor = compositor;
    d->client = client;
    d->init(client->client(), id, version);
    d->isInitialized = true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
    d->inputMethodControl = new WaylandInputMethodControl(this);
#endif
#endif
#ifndef QT_NO_DEBUG
    WaylandSurfacePrivate::removeUninitializedSurface(d);
#endif
}

/*!
 * Returns true if the WaylandSurface has been initialized.
 */
bool WaylandSurface::isInitialized() const
{
    Q_D(const WaylandSurface);
    return d->isInitialized;
}

/*!
 * \qmlproperty WaylandClient AuroraCompositor::WaylandSurface::client
 *
 * This property holds the client using this WaylandSurface.
 */

/*!
 * \property WaylandSurface::client
 *
 * This property holds the client using this WaylandSurface.
 */
WaylandClient *WaylandSurface::client() const
{
    Q_D(const WaylandSurface);
    if (isDestroyed() || !compositor() || !compositor()->clients().contains(d->client))
        return nullptr;

    return d->client;
}

/*!
 * Holds the \c wl_client using this WaylandSurface.
 */
::wl_client *WaylandSurface::waylandClient() const
{
    if (auto *c = client())
        return c->client();

    return nullptr;
}

/*!
 * \qmlproperty bool AuroraCompositor::WaylandSurface::hasContent
 *
 * This property holds whether the WaylandSurface has content.
 */

/*!
 * \property WaylandSurface::hasContent
 *
 * This property holds whether the WaylandSurface has content.
 */
bool WaylandSurface::hasContent() const
{
    Q_D(const WaylandSurface);
    return d->hasContent;
}

/*!
 * \qmlproperty rect AuroraCompositor::WaylandSurface::sourceGeometry
 * \since 5.13
 *
 * This property describes the portion of the attached Wayland buffer that should
 * be drawn on the screen. The coordinates are from the corner of the buffer and are
 * scaled by \l bufferScale.
 *
 * \sa bufferScale
 * \sa bufferSize
 * \sa destinationSize
 */

/*!
 * \property WaylandSurface::sourceGeometry
 * \since 5.13
 *
 * This property describes the portion of the attached WaylandBuffer that should
 * be drawn on the screen. The coordinates are from the corner of the buffer and are
 * scaled by \l bufferScale.
 *
 * \sa bufferScale
 * \sa bufferSize
 * \sa destinationSize
 */
QRectF WaylandSurface::sourceGeometry() const
{
    Q_D(const WaylandSurface);
    return d->sourceGeometry;
}

/*!
 * \qmlproperty size AuroraCompositor::WaylandSurface::destinationSize
 * \since 5.13
 *
 * This property holds the size of this WaylandSurface in surface coordinates.
 *
 * \sa bufferScale
 * \sa bufferSize
 */

/*!
 * \property WaylandSurface::destinationSize
 * \since 5.13
 *
 * This property holds the size of this WaylandSurface in surface coordinates.
 *
 * \sa bufferScale
 * \sa bufferSize
 */
QSize WaylandSurface::destinationSize() const
{
    Q_D(const WaylandSurface);
    return d->destinationSize;
}

/*!
 * \qmlproperty size AuroraCompositor::WaylandSurface::bufferSize
 *
 * This property holds the size of the current buffer of this WaylandSurface in pixels,
 * not in surface coordinates.
 *
 * For the size in surface coordinates, use \l destinationSize instead.
 *
 * \sa destinationSize
 * \sa bufferScale
 */

/*!
 * \property WaylandSurface::bufferSize
 *
 * This property holds the size of the current buffer of this WaylandSurface in pixels,
 * not in surface coordinates.
 *
 * For the size in surface coordinates, use \l destinationSize instead.
 *
 * \sa destinationSize
 * \sa bufferScale
 */
QSize WaylandSurface::bufferSize() const
{
    Q_D(const WaylandSurface);
    return d->bufferSize;
}

/*!
 * \qmlproperty size AuroraCompositor::WaylandSurface::bufferScale
 *
 * This property holds the WaylandSurface's buffer scale. The buffer scale lets
 * a client supply higher resolution buffer data for use on high resolution
 * outputs.
 */

/*!
 * \property WaylandSurface::bufferScale
 *
 * This property holds the WaylandSurface's buffer scale. The buffer scale
 * lets a client supply higher resolution buffer data for use on high
 * resolution outputs.
 */
int WaylandSurface::bufferScale() const
{
    Q_D(const WaylandSurface);
    return d->bufferScale;
}

/*!
 * \qmlproperty enum AuroraCompositor::WaylandSurface::contentOrientation
 *
 * This property holds the orientation of the WaylandSurface's contents.
 *
 * \sa {WaylandOutput::transform}{WaylandOutput.transform}
 */

/*!
 * \property WaylandSurface::contentOrientation
 *
 * This property holds the orientation of the WaylandSurface's contents.
 *
 * \sa WaylandOutput::transform
 */
Qt::ScreenOrientation WaylandSurface::contentOrientation() const
{
    Q_D(const WaylandSurface);
    return d->contentOrientation;
}

/*!
 * \enum WaylandSurface::Origin
 *
 * This enum type is used to specify the origin of a WaylandSurface's buffer.
 *
 * \value OriginTopLeft The origin is the top left corner of the buffer.
 * \value OriginBottomLeft The origin is the bottom left corner of the buffer.
 */

/*!
 * \qmlproperty enum AuroraCompositor::WaylandSurface::origin
 *
 * This property holds the origin of the WaylandSurface's buffer, or
 * WaylandSurface.OriginTopLeft if the surface has no buffer.
 *
 * It can have the following values:
 * \list
 * \li WaylandSurface.OriginTopLeft The origin is the top left corner of the buffer.
 * \li WaylandSurface.OriginBottomLeft The origin is the bottom left corner of the buffer.
 * \endlist
 */

/*!
 * \property WaylandSurface::origin
 *
 * This property holds the origin of the WaylandSurface's buffer, or
 * WaylandSurface::OriginTopLeft if the surface has no buffer.
 */
WaylandSurface::Origin WaylandSurface::origin() const
{
    Q_D(const WaylandSurface);
    return d->bufferRef.origin();
}

/*!
 * Returns the compositor for this WaylandSurface.
 */
WaylandCompositor *WaylandSurface::compositor() const
{
    Q_D(const WaylandSurface);
    return d->compositor;
}

/*!
 * Prepares all frame callbacks for sending.
 */
void WaylandSurface::frameStarted()
{
    Q_D(WaylandSurface);
    for (Internal::FrameCallback *c : qAsConst(d->frameCallbacks))
        c->canSend = true;
}

/*!
 * Sends pending frame callbacks.
 */
void WaylandSurface::sendFrameCallbacks()
{
    Q_D(WaylandSurface);
    uint time = d->compositor->currentTimeMsecs();
    int i = 0;
    while (i < d->frameCallbacks.size()) {
        if (d->frameCallbacks.at(i)->canSend) {
            d->frameCallbacks.at(i)->surface = nullptr;
            d->frameCallbacks.at(i)->send(time);
            d->frameCallbacks.removeAt(i);
        } else {
            i++;
        }
    }
}

/*!
 * Returns \c true if the WaylandSurface's input region contains the point \a p.
 * Otherwise returns \c false.
 */
bool WaylandSurface::inputRegionContains(const QPoint &p) const
{
    Q_D(const WaylandSurface);
    return d->inputRegion.contains(p);
}

/*!
 * Returns \c true if the WaylandSurface's input region contains the point \a position.
 * Otherwise returns \c false.
 *
 * \since 5.14
 */
bool WaylandSurface::inputRegionContains(const QPointF &position) const
{
    Q_D(const WaylandSurface);
    // QRegion::contains operates in integers. If a region has a rect (0,0,10,10), (0,0) is
    // inside while (10,10) is outside. Therefore, we can't use QPoint::toPoint(), which will
    // round upwards, meaning the point (-0.25,-0.25) would be rounded to (0,0) and count as
    // being inside the region, and similarly, a point (9.75,9.75) inside the region would be
    // rounded upwards and count as being outside the region.
    const QPoint floored(qFloor(position.x()), qFloor(position.y()));
    return d->inputRegion.contains(floored);
}

/*!
 * \qmlmethod void AuroraCompositor::WaylandSurface::destroy()
 *
 * Destroys the WaylandSurface.
 */

/*!
 * Destroys the WaylandSurface.
 */
void WaylandSurface::destroy()
{
    Q_D(WaylandSurface);
    d->deref();
}

/*!
 * \qmlmethod bool AuroraCompositor::WaylandSurface::isDestroyed()
 *
 * Returns \c true if the WaylandSurface has been destroyed. Otherwise returns \c false.
 */

/*!
 * Returns true if the WaylandSurface has been destroyed. Otherwise returns false.
 */
bool WaylandSurface::isDestroyed() const
{
    Q_D(const WaylandSurface);
    return d->destroyed;
}

/*!
 * \qmlproperty bool AuroraCompositor::WaylandSurface::cursorSurface
 *
 * This property holds whether the WaylandSurface is a cursor surface.
 */

/*!
 * \property WaylandSurface::cursorSurface
 *
 * This property holds whether the WaylandSurface is a cursor surface.
 */
void WaylandSurface::markAsCursorSurface(bool cursorSurface)
{
    Q_D(WaylandSurface);
    if (d->isCursorSurface == cursorSurface)
        return;

    d->isCursorSurface = cursorSurface;
    emit cursorSurfaceChanged();
}

bool WaylandSurface::isCursorSurface() const
{
    Q_D(const WaylandSurface);
    return d->isCursorSurface;
}

/*!
 * \qmlproperty bool AuroraCompositor::WaylandSurface::inhibitsIdle
 * \since 5.14
 *
 * This property holds whether this surface is intended to inhibit the idle
 * behavior of the compositor such as screen blanking, locking and screen saving.
 *
 * \sa IdleInhibitManagerV1
 */

/*!
 * \property WaylandSurface::inhibitsIdle
 * \since 5.14
 *
 * This property holds whether this surface is intended to inhibit the idle
 * behavior of the compositor such as screen blanking, locking and screen saving.
 *
 * \sa WaylandIdleInhibitManagerV1
 */
bool WaylandSurface::inhibitsIdle() const
{
    Q_D(const WaylandSurface);
    return !d->idleInhibitors.isEmpty();
}

/*!
 *  \qmlproperty bool AuroraCompositor::WaylandSurface::isOpaque
 *  \since 6.4
 *
 *  This property holds whether the surface is fully opaque, as reported by the
 *  client through the set_opaque_region request.
 */

/*!
 *  \property WaylandSurface::isOpaque
 *  \since 6.4
 *
 *  This property holds whether the surface is fully opaque, as reported by the
 *  client through the set_opaque_region request.
 */
bool WaylandSurface::isOpaque() const
{
    Q_D(const WaylandSurface);
    return d->isOpaque;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
WaylandInputMethodControl *WaylandSurface::inputMethodControl() const
{
    Q_D(const WaylandSurface);
    return d->inputMethodControl;
}
#endif
#endif

/*!
 * Updates the surface with the compositor's retained clipboard selection. Although
 * this is done automatically when the surface receives keyboard focus, this
 * function is useful for updating clients which do not have keyboard focus.
 */
#if QT_CONFIG(clipboard)
void WaylandSurface::updateSelection()
{
    Q_D(WaylandSurface);
    WaylandSeat *seat = d->compositor->defaultSeat();
    if (seat) {
        const Internal::DataDevice *dataDevice = WaylandSeatPrivate::get(seat)->dataDevice();
        if (dataDevice) {
            WaylandCompositorPrivate::get(d->compositor)->dataDeviceManager()->offerRetainedSelection(
                        dataDevice->resourceMap().value(d->resource()->client())->handle);
        }
    }
}
#endif

/*!
 * Returns this WaylandSurface's primary view.
 *
 * \sa WaylandView::advance(), WaylandSurface::setPrimaryView()
 */
WaylandView *WaylandSurface::primaryView() const
{
    Q_D(const WaylandSurface);
    if (d->views.isEmpty())
        return nullptr;
    return d->views.first();
}

/*!
 * Sets this WaylandSurface's primary view to \a view, in case there are
 * multiple views of this surface. The primary view is the view that
 * governs the client's refresh rate. It takes care of discarding buffer
 * references when WaylandView::advance() is called. See the documentation
 * for WaylandView::advance() for more details.
 *
 * In shell surface integrations, such as WaylandWlShellIntegration and
 * WaylandXdgShellV5Integration, maximize and fullscreen requests from the
 * client will only have an effect if the integration has the primary view
 * of the surface.
 *
 * \sa WaylandView::advance()
 */
void WaylandSurface::setPrimaryView(WaylandView *view)
{
    Q_D(WaylandSurface);

    if (!view)
        return;

    int index = d->views.indexOf(view);

    if (index < 0) {
        view->setSurface(this);
        index = d->views.indexOf(view);
    }

    d->views.move(index, 0);
}

/*!
 * Returns the views for this WaylandSurface.
 */
QList<WaylandView *> WaylandSurface::views() const
{
    Q_D(const WaylandSurface);
    return d->views;
}

/*!
 * Returns the WaylandSurface corresponding to the Wayland resource \a resource.
 */
WaylandSurface *WaylandSurface::fromResource(::wl_resource *resource)
{
    if (auto p = Internal::fromResource<WaylandSurfacePrivate *>(resource))
        return p->q_func();
    return nullptr;
}

/*!
 * Returns the Wayland resource corresponding to this WaylandSurface.
 */
struct wl_resource *WaylandSurface::resource() const
{
    Q_D(const WaylandSurface);
    return d->resource()->handle;
}

/*!
 * Sets a \a role on the surface. A role defines how a surface will be mapped on screen; without a
 * role a surface is supposed to be hidden. Once a role is assigned to a surface, this becomes its
 * permanent role. Any subsequent call to \c setRole() with a different role will trigger a
 * protocol error to the \a errorResource and send an \a errorCode to the client. Enforcing this
 * requirement is the main purpose of the surface role.
 *
 * The \a role is compared by pointer value. Any two objects of WaylandSurfaceRole will be
 * considered different roles, regardless of their names.
 *
 * The surface role is set internally by protocol implementations when a surface is adopted for a
 * specific purpose, for example in a \l{Shell Extensions - Qt Wayland Compositor}{shell extension}.
 * Unless you are developing extensions which use surfaces in this way, you should not call this
 * function.
 *
 * Returns true if a role can be assigned; false otherwise.
 */
bool WaylandSurface::setRole(WaylandSurfaceRole *role, wl_resource *errorResource, uint32_t errorCode)
{
    Q_D(WaylandSurface);

    if (d->role && d->role != role) {
            wl_resource_post_error(errorResource, errorCode,
                                   "Cannot assign role %s to wl_surface@%d, already has role %s\n",
                                   role->name().constData(), wl_resource_get_id(resource()),
                                   d->role->name().constData());
            return false;
    }

    d->role = role;
    return true;
}

WaylandSurfaceRole *WaylandSurface::role() const
{
    Q_D(const WaylandSurface);
    return d->role;
}

WaylandSurfacePrivate *WaylandSurfacePrivate::get(WaylandSurface *surface)
{
    return surface ? surface->d_func() : nullptr;
}

void WaylandSurfacePrivate::ref()
{
    ++refCount;
}

void WaylandSurfacePrivate::deref()
{
    if (--refCount == 0)
        WaylandCompositorPrivate::get(compositor)->destroySurface(q_func());
}

void WaylandSurfacePrivate::refView(WaylandView *view)
{
    if (views.contains(view))
        return;

    views.append(view);
    ref();
    view->bufferCommitted(bufferRef, QRect(QPoint(0,0), bufferRef.size()));
}

void WaylandSurfacePrivate::derefView(WaylandView *view)
{
    int nViews = views.removeAll(view);

    for (int i = 0; i < nViews && refCount > 0; i++) {
        deref();
    }
}

void WaylandSurfacePrivate::initSubsurface(WaylandSurface *parent, wl_client *client, int id, int version)
{
    Q_Q(WaylandSurface);
    WaylandSurface *oldParent = nullptr; // TODO: implement support for switching parents

    subsurface = new Subsurface(this);
    subsurface->init(client, id, version);
    subsurface->parentSurface = parent->d_func();
    emit q->parentChanged(parent, oldParent);
    emit parent->childAdded(q);
}

void WaylandSurfacePrivate::Subsurface::subsurface_set_position(wl_subsurface::Resource *resource, int32_t x, int32_t y)
{
    Q_UNUSED(resource);
    position = QPoint(x,y);
    emit surface->q_func()->subsurfacePositionChanged(position);

}

void WaylandSurfacePrivate::Subsurface::subsurface_place_above(wl_subsurface::Resource *resource, struct wl_resource *sibling)
{
    Q_UNUSED(resource);
    emit surface->q_func()->subsurfacePlaceAbove(WaylandSurface::fromResource(sibling));
}

void WaylandSurfacePrivate::Subsurface::subsurface_place_below(wl_subsurface::Resource *resource, struct wl_resource *sibling)
{
    Q_UNUSED(resource);
    emit surface->q_func()->subsurfacePlaceBelow(WaylandSurface::fromResource(sibling));
}

void WaylandSurfacePrivate::Subsurface::subsurface_set_sync(wl_subsurface::Resource *resource)
{
    Q_UNUSED(resource);
    // TODO: sync/desync implementation
    qDebug() << Q_FUNC_INFO;
}

void WaylandSurfacePrivate::Subsurface::subsurface_set_desync(wl_subsurface::Resource *resource)
{
    Q_UNUSED(resource);
    // TODO: sync/desync implementation
    qDebug() << Q_FUNC_INFO;
}

/*!
 * \qmlsignal AuroraCompositor::WaylandSurface::childAdded(WaylandSurface child)
 *
 * This signal is emitted when a wl_subsurface, \a child, has been added to the surface.
 */

/*!
 * \fn void WaylandSurface::childAdded(WaylandSurface *child)
 *
 * This signal is emitted when a wl_subsurface, \a child, has been added to the surface.
 */

/*!
 * \qmlsignal AuroraCompositor::WaylandSurface::surfaceDestroyed()
 *
 * This signal is emitted when the corresponding wl_surface is destroyed.
 */

/*!
 * \fn void WaylandSurface::surfaceDestroyed()
 *
 * This signal is emitted when the corresponing wl_surface is destroyed.
 */

/*!
 * \qmlsignal void AuroraCompositor::WaylandSurface::dragStarted(WaylandDrag drag)
 *
 * This signal is emitted when a \a drag has started from this surface.
 */

/*!
 * \fn void WaylandSurface::dragStarted(WaylandDrag *drag)
 *
 * This signal is emitted when a \a drag has started from this surface.
 */

/*!
 * \fn void damaged(const QRegion &rect)
 *
 * This signal is emitted when the client tells the compositor that a particular part of, or
 * possibly the entire surface has been updated, so the compositor can redraw that part.
 *
 * While the compositor APIs take care of redrawing automatically, this function may be useful
 * if you require a specific, custom behavior.
 */

/*!
 * \fn void parentChanged(WaylandSurface *newParent, WaylandSurface *oldParent)
 *
 * This signal is emitted when the client has requested that this surface should be a
 * subsurface of \a newParent.
 */

} // namespace Compositor

} // namespace Aurora
