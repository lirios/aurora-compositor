/****************************************************************************
**
** Copyright (C) 2017-2016 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include "aurorawaylandoutput.h"
#include "aurorawaylandoutput_p.h"

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandView>

#include <LiriAuroraCompositor/private/aurorawaylandsurface_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandcompositor_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandview_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandutils_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandxdgoutputv1_p.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QtMath>
#include <QtGui/QWindow>
#include <QtGui/QExposeEvent>
#include <QtGui/QScreen>
#include <private/qobject_p.h>

template<typename DPtr, typename Slot>
auto priv_slot(DPtr &&dptr, Slot &&slot)
{
    return [&dptr, &slot](auto && ... args)
    {
        (dptr->*slot)(std::forward<decltype(args)>(args) ...);
    };
}

namespace Aurora {

namespace Compositor {

static PrivateServer::wl_output::subpixel toWlSubpixel(const WaylandOutput::Subpixel &value)
{
    switch (value) {
    case WaylandOutput::SubpixelUnknown:
        return PrivateServer::wl_output::subpixel_unknown;
    case WaylandOutput::SubpixelNone:
        return PrivateServer::wl_output::subpixel_none;
    case WaylandOutput::SubpixelHorizontalRgb:
        return PrivateServer::wl_output::subpixel_horizontal_rgb;
    case WaylandOutput::SubpixelHorizontalBgr:
        return PrivateServer::wl_output::subpixel_horizontal_bgr;
    case WaylandOutput::SubpixelVerticalRgb:
        return PrivateServer::wl_output::subpixel_vertical_rgb;
    case WaylandOutput::SubpixelVerticalBgr:
        return PrivateServer::wl_output::subpixel_vertical_bgr;
    default:
        break;
    }

    return PrivateServer::wl_output::subpixel_unknown;
}

static PrivateServer::wl_output::transform toWlTransform(const WaylandOutput::Transform &value)
{
    switch (value) {
    case WaylandOutput::Transform90:
        return PrivateServer::wl_output::transform_90;
    case WaylandOutput::Transform180:
        return PrivateServer::wl_output::transform_180;
    case WaylandOutput::Transform270:
        return PrivateServer::wl_output::transform_270;
    case WaylandOutput::TransformFlipped:
        return PrivateServer::wl_output::transform_flipped;
    case WaylandOutput::TransformFlipped90:
        return PrivateServer::wl_output::transform_flipped_90;
    case WaylandOutput::TransformFlipped180:
        return PrivateServer::wl_output::transform_flipped_180;
    case WaylandOutput::TransformFlipped270:
        return PrivateServer::wl_output::transform_flipped_270;
    default:
        break;
    }

    return PrivateServer::wl_output::transform_normal;
}

WaylandOutputPrivate::WaylandOutputPrivate(WaylandOutput *self)
    : q_ptr(self)
{
}

WaylandOutputPrivate::~WaylandOutputPrivate()
{
}

void WaylandOutputPrivate::output_bind_resource(Resource *resource)
{
    sendGeometry(resource);

    for (const WaylandOutputMode &mode : modes)
        sendMode(resource, mode);

    if (resource->version() >= 2) {
        send_scale(resource->handle, scaleFactor);
        send_done(resource->handle);
    }
}

void WaylandOutputPrivate::_q_handleMaybeWindowPixelSizeChanged()
{
    if (!window)
        return;

    const QSize pixelSize = window->size() * window->devicePixelRatio();

    if (pixelSize != windowPixelSize) {
        windowPixelSize = pixelSize;
        handleWindowPixelSizeChanged();
    }
}

void WaylandOutputPrivate::_q_handleWindowDestroyed()
{
    Q_Q(WaylandOutput);
    window = nullptr;
    emit q->windowChanged();
    emit q->windowDestroyed();
}

void WaylandOutputPrivate::sendGeometry(const Resource *resource)
{
    send_geometry(resource->handle,
                  position.x(), position.y(),
                  physicalSize.width(), physicalSize.height(),
                  toWlSubpixel(subpixel), manufacturer, model,
                  toWlTransform(transform));
}

void WaylandOutputPrivate::sendGeometryInfo()
{
    for (const Resource *resource : resourceMap().values()) {
        sendGeometry(resource);
        if (resource->version() >= 2)
            send_done(resource->handle);
    }

    if (xdgOutput)
        WaylandXdgOutputV1Private::get(xdgOutput)->sendDone();
}

void WaylandOutputPrivate::sendMode(const Resource *resource, const WaylandOutputMode &mode)
{
    quint32 flags = 0;
    if (currentMode == modes.indexOf(mode))
        flags |= PrivateServer::wl_output::mode_current;
    if (preferredMode == modes.indexOf(mode))
        flags |= PrivateServer::wl_output::mode_preferred;

    send_mode(resource->handle, flags,
              mode.size().width(), mode.size().height(),
              mode.refreshRate());
}

void WaylandOutputPrivate::sendModesInfo()
{
    for (const Resource *resource : resourceMap().values()) {
        for (const WaylandOutputMode &mode : modes)
            sendMode(resource, mode);
        if (resource->version() >= 2)
            send_done(resource->handle);
    }

    if (xdgOutput)
        WaylandXdgOutputV1Private::get(xdgOutput)->sendDone();
}

void WaylandOutputPrivate::handleWindowPixelSizeChanged()
{
    Q_Q(WaylandOutput);
    Q_ASSERT(window);
    if (sizeFollowsWindow && currentMode <= modes.size() - 1) {
        if (currentMode >= 0) {
            WaylandOutputMode mode = modes.at(currentMode);
            mode.setSize(windowPixelSize);
            modes.replace(currentMode, mode);
            emit q->geometryChanged();
            if (!availableGeometry.isValid())
                emit q->availableGeometryChanged();
            sendModesInfo();
        } else {
            // We didn't add a mode during the initialization because the window
            // size was invalid, let's add it now
            int mHzRefreshRate = qFloor(window->screen()->refreshRate() * 1000);
            WaylandOutputMode mode(windowPixelSize, mHzRefreshRate);
            if (mode.isValid()) {
                modes.clear();
                q->addMode(mode, true);
                q->setCurrentMode(mode);
            }
        }
    }
}

void WaylandOutputPrivate::addView(WaylandView *view, WaylandSurface *surface)
{
    for (int i = 0; i < surfaceViews.size(); i++) {
        if (surface == surfaceViews.at(i).surface) {
            if (!surfaceViews.at(i).views.contains(view)) {
                surfaceViews[i].views.append(view);
            }
            return;
        }
    }

    surfaceViews.append(WaylandSurfaceViewMapper(surface,view));
}

void WaylandOutputPrivate::removeView(WaylandView *view, WaylandSurface *surface)
{
    Q_Q(WaylandOutput);
    for (int i = 0; i < surfaceViews.size(); i++) {
        if (surface == surfaceViews.at(i).surface) {
            bool removed = surfaceViews[i].views.removeOne(view);
            if (surfaceViews.at(i).views.isEmpty() && removed) {
                if (surfaceViews.at(i).has_entered)
                    q->surfaceLeave(surface);
                surfaceViews.remove(i);
            }
            return;
        }
    }
    qWarning("%s Could not find view %p for surface %p to remove. Possible invalid state", Q_FUNC_INFO, view, surface);
}

WaylandOutput::WaylandOutput()
    : WaylandObject()
    , d_ptr(new WaylandOutputPrivate(this))
{
}

/*!
   \qmltype WaylandOutput
   \instantiates WaylandOutput
   \inqmlmodule Aurora.Compositor
   \since 5.8
   \brief Provides access to a displayable area managed by the compositor.

   The WaylandOutput manages a rectangular area within bounds of the compositor's
   geometry, to use it for displaying client content. This could, for instance, be
   a screen managed by the WaylandCompositor.

   The type corresponds to the \c wl_output interface in the Wayland protocol.

   \note If the compositor has multiple Wayland outputs, the \l Qt::AA_ShareOpenGLContexts
   attribute must be set before the \l QGuiApplication object is constructed.
*/

/*!
   \class WaylandOutput
   \inmodule AuroraCompositor
   \since 5.8
   \brief The WaylandOutput class represents a displayable area managed by the compositor.

   The WaylandOutput manages a rectangular area within bounds of the compositor's
   geometry, to use it for displaying client content. This could, for instance, be
   a screen managed by the WaylandCompositor.

   The class corresponds to the \c wl_output interface in the Wayland protocol.
*/

/*!
 * Constructs a WaylandOutput in \a compositor and with the specified \a window. The
 * \l{WaylandCompositor::create()}{create()} function must be called on the
 * \a compositor before constructing a WaylandOutput for it.
 *
 * The WaylandOutput object is initialized later, in reaction to an event.
 * At this point it is added as an output for the \a compositor. If it is the
 * first WaylandOutput object created for this \a compositor, it becomes the
 * \l{WaylandCompositor::defaultOutput()}{default output}.
 */
WaylandOutput::WaylandOutput(WaylandCompositor *compositor, QWindow *window)
    : WaylandObject()
    , d_ptr(new WaylandOutputPrivate(this))
{
    Q_D(WaylandOutput);
    d->compositor = compositor;
    d->window = window;
    WaylandCompositorPrivate::get(compositor)->addPolishObject(this);
}

/*!
 * Destroys the WaylandOutput.
 */
WaylandOutput::~WaylandOutput()
{
    Q_D(WaylandOutput);
    if (d->compositor)
        WaylandCompositorPrivate::get(d->compositor)->removeOutput(this);
}

/*!
 * \internal
 */
void WaylandOutput::initialize()
{
    Q_D(WaylandOutput);

    Q_ASSERT(!d->initialized);
    Q_ASSERT(d->compositor);
    Q_ASSERT(d->compositor->isCreated());

    if (!d->window && d->sizeFollowsWindow) {
        qWarning("Setting WaylandOutput::sizeFollowsWindow without a window has no effect");
    }

    // Replace modes with one that follows the window size and refresh rate,
    // but only if window size is valid
    if (d->window && d->sizeFollowsWindow) {
        WaylandOutputMode mode(d->window->size() * d->window->devicePixelRatio(),
                                qFloor(d->window->screen()->refreshRate() * 1000));
        if (mode.isValid()) {
            d->modes.clear();
            addMode(mode, true);
            setCurrentMode(mode);
        }
    }

    WaylandCompositorPrivate::get(d->compositor)->addOutput(this);

    if (d->window) {
        connect(d->window, SIGNAL(widthChanged(int)), this, SLOT(_q_handleMaybeWindowPixelSizeChanged()));
        connect(d->window, SIGNAL(heightChanged(int)), this, SLOT(_q_handleMaybeWindowPixelSizeChanged()));
        connect(d->window, SIGNAL(screenChanged(QScreen*)), this, SLOT(_q_handleMaybeWindowPixelSizeChanged()));
        connect(d->window, SIGNAL(destroyed()), this, SLOT(_q_handleWindowDestroyed()));
    }

    d->init(d->compositor->display(), 2);

    d->initialized = true;
}

/*!
 * Returns the WaylandOutput corresponding to \a resource.
 */
WaylandOutput *WaylandOutput::fromResource(wl_resource *resource)
{
    if (auto p = Internal::fromResource<WaylandOutputPrivate *>(resource))
        return p->q_func();
    return nullptr;
}

/*!
 * \internal
 */
struct ::wl_resource *WaylandOutput::resourceForClient(WaylandClient *client) const
{
    Q_D(const WaylandOutput);
    WaylandOutputPrivate::Resource *r = d->resourceMap().value(client->client());
    if (r)
        return r->handle;

    return nullptr;
}

/*!
 * Schedules a QEvent::UpdateRequest to be delivered to the WaylandOutput's \l{window()}{window}.
 *
 * \sa QWindow::requestUpdate()
 */
void WaylandOutput::update()
{
    Q_D(WaylandOutput);
    if (!d->window)
        return;
    d->window->requestUpdate();
}

/*!
 * \qmlproperty WaylandCompositor AuroraCompositor::WaylandOutput::compositor
 *
 * This property holds the compositor displaying content on this WaylandOutput.
 *
 * \note This property can be set only once, before the WaylandOutput component
 * is completed.
 */

/*!
 * Returns the compositor for this WaylandOutput.
 */
WaylandCompositor *WaylandOutput::compositor() const
{
    return d_func()->compositor;
}

/*!
 * \internal
 */
void WaylandOutput::setCompositor(WaylandCompositor *compositor)
{
    Q_D(WaylandOutput);

    if (d->compositor == compositor)
        return;

    if (d->initialized) {
        qWarning("Setting WaylandCompositor %p on WaylandOutput %p is not supported after WaylandOutput has been initialized\n", compositor, this);
        return;
    }
    if (d->compositor && d->compositor != compositor) {
        qWarning("Possible initialization error. Moving WaylandOutput %p between compositor instances.\n", this);
    }

    d->compositor = compositor;

    WaylandCompositorPrivate::get(compositor)->addPolishObject(this);
}

/*!
 * \qmlproperty string AuroraCompositor::WaylandOutput::manufacturer
 *
 * This property holds a textual description of the manufacturer of this WaylandOutput.
 */

/*!
 * \property WaylandOutput::manufacturer
 *
 * This property holds a textual description of the manufacturer of this WaylandOutput.
 */
QString WaylandOutput::manufacturer() const
{
    return d_func()->manufacturer;
}

void WaylandOutput::setManufacturer(const QString &manufacturer)
{
    Q_D(WaylandOutput);

    if (d->manufacturer == manufacturer)
        return;

    d->manufacturer = manufacturer;
    d->sendGeometryInfo();
    Q_EMIT manufacturerChanged();
}

/*!
 * \qmlproperty string AuroraCompositor::WaylandOutput::model
 *
 * This property holds a textual description of the model of this WaylandOutput.
 */

/*!
 * \property WaylandOutput::model
 *
 * This property holds a textual description of the model of this WaylandOutput.
 */
QString WaylandOutput::model() const
{
    return d_func()->model;
}

void WaylandOutput::setModel(const QString &model)
{
    Q_D(WaylandOutput);

    if (d->model == model)
        return;

    d->model = model;
    d->sendGeometryInfo();
    Q_EMIT modelChanged();
}

/*!
 * \qmlproperty point AuroraCompositor::WaylandOutput::position
 *
 * This property holds the position of this WaylandOutput in the compositor's coordinate system.
 */

/*!
 * \property WaylandOutput::position
 *
 * This property holds the position of this WaylandOutput in the compositor's coordinate system.
 */
QPoint WaylandOutput::position() const
{
    return d_func()->position;
}

void WaylandOutput::setPosition(const QPoint &pt)
{
    Q_D(WaylandOutput);
    if (d->position == pt)
        return;

    d->position = pt;

    d->sendGeometryInfo();

    Q_EMIT positionChanged();
    Q_EMIT geometryChanged();
}

/*!
 * Returns the list of modes.
 */
QList<WaylandOutputMode> WaylandOutput::modes() const
{
    Q_D(const WaylandOutput);
    return d->modes.toList();
}

/*!
 * Adds the mode \a mode to the output and mark it as preferred
 * if \a preferred is \c true.
 * Please note there can only be one preferred mode.
 */
void WaylandOutput::addMode(const WaylandOutputMode &mode, bool preferred)
{
    Q_D(WaylandOutput);

    if (!mode.isValid()) {
        qWarning("Cannot add an invalid mode");
        return;
    }

    if (d->modes.indexOf(mode) < 0)
        d->modes.append(mode);

    if (preferred)
        d->preferredMode = d->modes.indexOf(mode);

    emit modeAdded();
}

/*!
 * Returns the output's size in pixels and refresh rate in mHz.
 * If the current mode is not set it will return an invalid mode.
 *
 * \sa WaylandOutput::modes
 * \sa WaylandOutputMode
 */
WaylandOutputMode WaylandOutput::currentMode() const
{
    Q_D(const WaylandOutput);

    if (d->currentMode >= 0 && d->currentMode <= d->modes.size() - 1)
        return d->modes.at(d->currentMode);
    return WaylandOutputMode();
}

/*!
 * Sets the current mode.
 * The mode \a mode must have been previously added.
 *
 * \sa WaylandOutput::modes
 * \sa WaylandOutputMode
 */
void WaylandOutput::setCurrentMode(const WaylandOutputMode &mode)
{
    Q_D(WaylandOutput);

    int index = d->modes.indexOf(mode);
    if (index < 0) {
        qWarning("Cannot set an unknown WaylandOutput mode as current");
        return;
    }

    d->currentMode = index;

    Q_EMIT currentModeChanged();
    Q_EMIT geometryChanged();
    if (!d->availableGeometry.isValid())
        emit availableGeometryChanged();

    d->sendModesInfo();
}

/*!
 * \qmlproperty rect AuroraCompositor::WaylandOutput::geometry
 *
 * This property holds the geometry of the WaylandOutput.
 */

/*!
 * \property WaylandOutput::geometry
 *
 * This property holds the geometry of the WaylandOutput.
 *
 * \sa WaylandOutput::currentMode
 */
QRect WaylandOutput::geometry() const
{
    Q_D(const WaylandOutput);
    return QRect(d->position, currentMode().size());
}

/*!
 * \qmlproperty rect AuroraCompositor::WaylandOutput::availableGeometry
 *
 * This property holds the geometry of the WaylandOutput available for displaying content.
 * The available geometry is in output coordinates space, starts from 0,0 and it's as big
 * as the output by default.
 *
 * \sa WaylandOutput::geometry
 */

/*!
 * \property WaylandOutput::availableGeometry
 *
 * This property holds the geometry of the WaylandOutput available for displaying content.
 * The available geometry is in output coordinates space, starts from 0,0 and it's as big
 * as the output by default.
 *
 * \sa WaylandOutput::currentMode, WaylandOutput::geometry
 */
QRect WaylandOutput::availableGeometry() const
{
    Q_D(const WaylandOutput);

    if (!d->availableGeometry.isValid())
        return QRect(QPoint(0, 0), currentMode().size());

    return d->availableGeometry;
}

void WaylandOutput::setAvailableGeometry(const QRect &availableGeometry)
{
    Q_D(WaylandOutput);
    if (d->availableGeometry == availableGeometry)
        return;

    if (availableGeometry.topLeft().x() < 0 || availableGeometry.topLeft().y() < 0)
        qWarning("Available geometry should be a portion of the output");

    d->availableGeometry = availableGeometry;

    Q_EMIT availableGeometryChanged();
}

/*!
 * \qmlproperty size AuroraCompositor::WaylandOutput::physicalSize
 *
 * This property holds the physical size of the WaylandOutput in millimeters.
 *
 * \sa WaylandOutput::geometry
 */

/*!
 * \property WaylandOutput::physicalSize
 *
 * This property holds the physical size of the WaylandOutput in millimeters.
 *
 * \sa WaylandOutput::geometry, WaylandOutput::currentMode
 */
QSize WaylandOutput::physicalSize() const
{
    return d_func()->physicalSize;
}

void WaylandOutput::setPhysicalSize(const QSize &size)
{
    Q_D(WaylandOutput);
    if (d->physicalSize == size)
        return;

    d->physicalSize = size;

    d->sendGeometryInfo();

    Q_EMIT physicalSizeChanged();
}

/*!
 * \enum WaylandOutput::Subpixel
 *
 * This enum type is used to specify the subpixel arrangement of a WaylandOutput.
 *
 * \value SubpixelUnknown The subpixel arrangement is not set.
 * \value SubpixelNone There are no subpixels.
 * \value SubpixelHorizontalRgb The subpixels are arranged horizontally in red, green, blue order.
 * \value SubpixelHorizontalBgr The subpixels are arranged horizontally in blue, green, red order.
 * \value SubpixelVerticalRgb The subpixels are arranged vertically in red, green, blue order.
 * \value SubpixelVerticalBgr The subpixels are arranged vertically in blue, green, red order.
 *
 * \sa WaylandOutput::subpixel
 */

/*!
 * \qmlproperty enum AuroraCompositor::WaylandOutput::subpixel
 *
 * This property holds the subpixel arrangement of this WaylandOutput.
 *
 * \list
 * \li WaylandOutput.SubpixelUnknown The subpixel arrangement is not set.
 * \li WaylandOutput.SubpixelNone There are no subpixels.
 * \li WaylandOutput.SubpixelHorizontalRgb The subpixels are arranged horizontally in red, green, blue order.
 * \li WaylandOutput.SubpixelHorizontalBgr The subpixels are arranged horizontally in blue, green, red order.
 * \li WaylandOutput.SubpixelVerticalRgb The subpixels are arranged vertically in red, green, blue order.
 * \li WaylandOutput.SubpixelVerticalBgr The subpixels are arranged vertically in blue, green, red order.
 * \endlist
 *
 * The default is WaylandOutput.SubpixelUnknown.
 */

/*!
 * \property WaylandOutput::subpixel
 *
 * This property holds the subpixel arrangement of this WaylandOutput. The default is
 * WaylandOutput::SubpixelUnknown.
 */
WaylandOutput::Subpixel WaylandOutput::subpixel() const
{
    return d_func()->subpixel;
}

void WaylandOutput::setSubpixel(const Subpixel &subpixel)
{
    Q_D(WaylandOutput);
    if (d->subpixel == subpixel)
        return;

    d->subpixel = subpixel;

    d->sendGeometryInfo();

    Q_EMIT subpixelChanged();
}

/*! \enum WaylandOutput::Transform
 *
 * This enum type is used to specify the orientation of a WaylandOutput.
 *
 * \value TransformNormal The orientation is normal.
 * \value Transform90 The orientation is rotated 90 degrees.
 * \value Transform180 The orientation is rotated 180 degrees.
 * \value Transform270 The orientation is rotated 270 degrees.
 * \value TransformFlipped The orientation is mirrored.
 * \value TransformFlipped90 The orientation is mirrored, and rotated 90 degrees.
 * \value TransformFlipped180 The orientation is mirrored, and rotated 180 degrees.
 * \value TransformFlipped270 The orientation is mirrored, and rotated 270 degrees.
 *
 * \sa WaylandOutput::transform
*/

/*!
 * \qmlproperty enum AuroraCompositor::WaylandOutput::transform
 *
 * This property holds the transformation that the WaylandCompositor applies to a surface
 * to compensate for the orientation of the WaylandOutput.
 *
 * \list
 * \li WaylandOutput.TransformNormal The orientation is normal.
 * \li WaylandOutput.Transform90 The orientation is rotated 90 degrees.
 * \li WaylandOutput.Transform180 The orientation is rotated 180 degrees.
 * \li WaylandOutput.Transform270 The orientation is rotated 270 degrees.
 * \li WaylandOutput.TransformFlipped The orientation is mirrored.
 * \li WaylandOutput.TransformFlipped90 The orientation is mirrored, then rotated 90 degrees.
 * \li WaylandOutput.TransformFlipped180 The orientation is mirrored, then rotated 180 degrees.
 * \li WaylandOutput.TransformFlipped270 The orientation is mirrored, then rotated 270 degrees.
 * \endlist
 *
 * The default is WaylandOutput.TransformNormal.
 */

/*!
 * \property WaylandOutput::transform
 *
 * This property holds the transformation that the WaylandCompositor applies to a surface
 * to compensate for the orientation of the WaylandOutput.
 *
 * The default is WaylandOutput::TransformNormal.
 */
WaylandOutput::Transform WaylandOutput::transform() const
{
    return d_func()->transform;
}

void WaylandOutput::setTransform(const Transform &transform)
{
    Q_D(WaylandOutput);
    if (d->transform == transform)
        return;

    d->transform = transform;

    d->sendGeometryInfo();

    Q_EMIT transformChanged();
}

/*!
 * \qmlproperty int AuroraCompositor::WaylandOutput::scaleFactor
 *
 * This property holds the factor by which the WaylandCompositor scales surface buffers
 * before they are displayed. It is used on high density output devices where unscaled content
 * would be too small to be practical. The client can in turn set the scale factor of its
 * buffer to match the output if it prefers to provide high resolution content that is
 * suitable for the output device.
 *
 * The default is 1 (no scaling).
 */

/*!
 * \property WaylandOutput::scaleFactor
 *
 * This property holds the factor by which the WaylandCompositor scales surface buffers
 * before they are displayed. This is used on high density output devices where unscaled content
 * would be too small to be practical. The client can in turn set the scale factor of its
 * buffer to match the output if it prefers to provide high resolution content that is
 * suitable for the output device.
 *
 * The default is 1 (no scaling).
 */
int WaylandOutput::scaleFactor() const
{
    return d_func()->scaleFactor;
}

void WaylandOutput::setScaleFactor(int scale)
{
    Q_D(WaylandOutput);
    if (d->scaleFactor == scale)
        return;

    d->scaleFactor = scale;

    const auto resMap = d->resourceMap();
    for (WaylandOutputPrivate::Resource *resource : resMap) {
        if (resource->version() >= 2) {
            d->send_scale(resource->handle, scale);
            d->send_done(resource->handle);
        }
    }

    Q_EMIT scaleFactorChanged();

    if (d->xdgOutput)
        WaylandXdgOutputV1Private::get(d->xdgOutput)->sendDone();
}

/*!
 * \qmlproperty bool AuroraCompositor::WaylandOutput::sizeFollowsWindow
 *
 * This property controls whether the size of the WaylandOutput matches the
 * size of its window.
 *
 * If this property is true, all modes previously added are replaced by a
 * mode that matches window size and screen refresh rate.
 *
 * The default is false.
 */

/*!
 * \property WaylandOutput::sizeFollowsWindow
 *
 * This property controls whether the size of the WaylandOutput matches the
 * size of its window.
 *
 * If this property is true, all modes previously added are replaced by a
 * mode that matches window size and screen refresh rate.
 *
 * The default is false.
 */
bool WaylandOutput::sizeFollowsWindow() const
{
    return d_func()->sizeFollowsWindow;
}

void WaylandOutput::setSizeFollowsWindow(bool follow)
{
    Q_D(WaylandOutput);

    if (follow != d->sizeFollowsWindow) {
        d->sizeFollowsWindow = follow;
        Q_EMIT sizeFollowsWindowChanged();
    }
}

/*!
 * \qmlproperty Window AuroraCompositor::WaylandOutput::window
 *
 * This property holds the Window for this WaylandOutput.
 *
 * \note This property can be set only once, before the WaylandOutput
 * component is completed.
 */

/*!
 * \property WaylandOutput::window
 *
 * This property holds the QWindow for this WaylandOutput.
 */
QWindow *WaylandOutput::window() const
{
    return d_func()->window;
}

void WaylandOutput::setWindow(QWindow *window)
{
    Q_D(WaylandOutput);
    if (d->window == window)
        return;
    if (d->initialized) {
        qWarning("Setting QWindow %p on WaylandOutput %p is not supported after WaylandOutput has been initialized\n", window, this);
        return;
    }
    d->window = window;
    emit windowChanged();
}

/*!
 * Informs WaylandOutput that a frame has started.
 */
void WaylandOutput::frameStarted()
{
    Q_D(WaylandOutput);
    for (int i = 0; i < d->surfaceViews.size(); i++) {
        WaylandSurfaceViewMapper &surfacemapper = d->surfaceViews[i];
        if (surfacemapper.maybePrimaryView())
            surfacemapper.surface->frameStarted();
    }
}

/*!
 * Sends pending frame callbacks.
 */
void WaylandOutput::sendFrameCallbacks()
{
    Q_D(WaylandOutput);
    for (int i = 0; i < d->surfaceViews.size(); i++) {
        const WaylandSurfaceViewMapper &surfacemapper = d->surfaceViews.at(i);
        if (surfacemapper.surface && surfacemapper.surface->hasContent()) {
            if (!surfacemapper.has_entered) {
                surfaceEnter(surfacemapper.surface);
                d->surfaceViews[i].has_entered = true;
            }
            if (auto primaryView = surfacemapper.maybePrimaryView()) {
                if (!WaylandViewPrivate::get(primaryView)->independentFrameCallback)
                    surfacemapper.surface->sendFrameCallbacks();
            }
        }
    }
    wl_display_flush_clients(d->compositor->display());
}

/*!
 * \internal
 */
void WaylandOutput::surfaceEnter(WaylandSurface *surface)
{
    if (!surface)
        return;

    auto clientResource = resourceForClient(surface->client());
    if (clientResource)
        WaylandSurfacePrivate::get(surface)->send_enter(clientResource);
}

/*!
 * \internal
 */
void WaylandOutput::surfaceLeave(WaylandSurface *surface)
{
    if (!surface || !surface->client())
        return;

    auto *clientResource = resourceForClient(surface->client());
    if (clientResource)
        WaylandSurfacePrivate::get(surface)->send_leave(clientResource);
}

/*!
 * \internal
 */
bool WaylandOutput::event(QEvent *event)
{
    if (event->type() == QEvent::Polish)
        initialize();
    return QObject::event(event);
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandoutput.cpp"
