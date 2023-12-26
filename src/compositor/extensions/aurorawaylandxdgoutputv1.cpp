// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <LiriAuroraCompositor/WaylandCompositor>

#include "aurorawaylandxdgoutputv1_p.h"
#include "aurorawaylandoutput_p.h"

#include <wayland-server.h>

namespace Aurora {

namespace Compositor {

/*!
 * \qmltype XdgOutputManagerV1
 * \instantiates WaylandXdgOutputManagerV1
 * \inqmlmodule Aurora.Compositor.XdgShell
 * \since 5.14
 * \brief Provides an extension for describing outputs in a desktop oriented fashion.
 *
 * The XdgOutputManagerV1 extension provides a way for a compositor to describe outputs in a way
 * that is more in line with the concept of an output on desktop oriented systems.
 *
 * Some information may not make sense in other applications such as IVI systems.
 *
 * Typically the global compositor space on a desktop system is made of a
 * contiguous or overlapping set of rectangular regions.
 *
 * XdgOutputManagerV1 corresponds to the Wayland interface, \c zxdg_output_manager_v1.
 *
 * To provide the functionality of the extension in a compositor, create an instance of the
 * XdgOutputManagerV1 component and add it to the list of extensions supported by the compositor,
 * and associated each XdgOutputV1 with its WaylandOutput:
 *
 * \qml
 * import Aurora.Compositor
 *
 * WaylandCompositor {
 *     XdgOutputManagerV1 {
 *         WaylandOutput {
 *             id: output1
 *
 *             position: Qt.point(0, 0)
 *             window: Window {}
 *
 *             XdgOutputV1 {
 *                 name: "WL-1"
 *                 logicalPosition: output1.position
 *                 logicalSize: Qt.size(output1.geometry.width / output1.scaleFactor,
 *                                      output1.geometry.height / output1.scaleFactor)
 *             }
 *         }
 *
 *         WaylandOutput {
 *             id: output2
 *
 *             position: Qt.point(800, 0)
 *             window: Window {}
 *
 *             XdgOutputV1 {
 *                 name: "WL-2"
 *                 logicalPosition: output2.position
 *                 logicalSize: Qt.size(output2.geometry.width / output2.scaleFactor,
 *                                      output2.geometry.height / output2.scaleFactor)
 *             }
 *         }
 *     }
 * }
 * \endqml
 */

/*!
 * \class WaylandXdgOutputManagerV1
 * \inmodule AuroraCompositor
 * \since 5.14
 * \brief Provides an extension for describing outputs in a desktop oriented fashion.
 *
 * The WaylandXdgOutputManagerV1 extension provides a way for a compositor to describe outputs in a way
 * that is more in line with the concept of an output on desktop oriented systems.
 *
 * Some information may not make sense in other applications such as IVI systems.
 *
 * WaylandXdgOutputManagerV1 corresponds to the Wayland interface, \c zxdg_output_manager_v1.
 */

/*!
 * Constructs a WaylandXdgOutputManagerV1 object.
 */
WaylandXdgOutputManagerV1::WaylandXdgOutputManagerV1()
    : WaylandCompositorExtensionTemplate<WaylandXdgOutputManagerV1>(*new WaylandXdgOutputManagerV1Private())
{
}

/*!
 * Constructs a WaylandXdgOutputManagerV1 object for the provided \a compositor.
 */
WaylandXdgOutputManagerV1::WaylandXdgOutputManagerV1(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate<WaylandXdgOutputManagerV1>(compositor, *new WaylandXdgOutputManagerV1Private())
{
}

// WaylandXdgOutputManagerV1Private

/*!
 * Initializes the extension.
 */
void WaylandXdgOutputManagerV1::initialize()
{
    Q_D(WaylandXdgOutputManagerV1);

    WaylandCompositorExtensionTemplate::initialize();
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qCWarning(gLcAuroraCompositor) << "Failed to find WaylandCompositor when initializing WaylandXdgOutputManagerV1";
        return;
    }
    d->init(compositor->display(), d->interfaceVersion());
}

/*!
 * Returns the Wayland interface for WaylandXdgOutputManagerV1.
 */
const wl_interface *WaylandXdgOutputManagerV1::interface()
{
    return WaylandXdgOutputManagerV1Private::interface();
}

// WaylandXdgOutputManagerV1Private

void WaylandXdgOutputManagerV1Private::registerXdgOutput(WaylandOutput *output, WaylandXdgOutputV1 *xdgOutput)
{
    if (!xdgOutputs.contains(output)) {
        xdgOutputs[output] = xdgOutput;
        WaylandOutputPrivate::get(output)->xdgOutput = xdgOutput;
    }
}

void WaylandXdgOutputManagerV1Private::unregisterXdgOutput(WaylandOutput *output)
{
    xdgOutputs.remove(output);
}

void WaylandXdgOutputManagerV1Private::zxdg_output_manager_v1_get_xdg_output(Resource *resource,
                                                                              uint32_t id,
                                                                              wl_resource *outputResource)
{
    Q_Q(WaylandXdgOutputManagerV1);

    // Verify if the associated output exist
    auto *output = WaylandOutput::fromResource(outputResource);
    if (!output) {
        qCWarning(gLcAuroraCompositor,
                  "The client is requesting a WaylandXdgOutputV1 for a "
                  "WaylandOutput that doesn't exist");
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT, "output not found");
        return;
    }

    // Do we have a WaylandXdgOutputV1 for this output?
    if (!xdgOutputs.contains(output)) {
        qCWarning(gLcAuroraCompositor,
                  "The client is requesting a WaylandXdgOutputV1 that the compositor "
                  "didn't create before");
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "compositor didn't create a WaylandXdgOutputV1 for this zxdg_output_v1 object");
        return;
    }

    // Bind WaylandXdgOutputV1 and initialize
    auto *xdgOutput = xdgOutputs[output];
    auto *xdgOutputPrivate = WaylandXdgOutputV1Private::get(xdgOutput);
    Q_ASSERT(xdgOutputPrivate);
    xdgOutputPrivate->setManager(q);
    xdgOutputPrivate->setOutput(output);
    xdgOutputPrivate->add(resource->client(), id, qMin(resource->version(), WaylandXdgOutputV1Private::interfaceVersion()));
}

// WaylandXdgOutputV1

WaylandXdgOutputV1::WaylandXdgOutputV1()
    : QObject(*new WaylandXdgOutputV1Private)
{
}

WaylandXdgOutputV1::WaylandXdgOutputV1(WaylandOutput *output, WaylandXdgOutputManagerV1 *manager)
    : QObject(*new WaylandXdgOutputV1Private)
{
    Q_D(WaylandXdgOutputV1);

    // Set members before emitting changed signals so that handlers will
    // see both already set and not nullptr, avoiding potential crashes
    d->manager = manager;
    d->output = output;

    WaylandXdgOutputManagerV1Private::get(d->manager)->registerXdgOutput(output, this);

    emit managerChanged();
    emit outputChanged();
}

WaylandXdgOutputV1::~WaylandXdgOutputV1()
{
    Q_D(WaylandXdgOutputV1);

    if (d->manager)
        WaylandXdgOutputManagerV1Private::get(d->manager)->unregisterXdgOutput(d->output);
}

/*!
 * \qmlproperty XdgOutputManagerV1 XdgOutputV1::manager
 * \readonly
 *
 * This property holds the object that manages this XdgOutputV1.
 */
/*!
 * \property WaylandXdgOutputV1::manager
 * \readonly
 *
 * This property holds the object that manages this WaylandXdgOutputV1.
 */
WaylandXdgOutputManagerV1 *WaylandXdgOutputV1::manager() const
{
    Q_D(const WaylandXdgOutputV1);
    return d->manager;
}

/*!
 * \qmlproperty WaylandOutput XdgOutputV1::output
 * \readonly
 *
 * This property holds the WaylandOutput associated with this XdgOutputV1.
 */
/*!
 * \property WaylandXdgOutputV1::output
 * \readonly
 *
 * This property holds the WaylandOutput associated with this WaylandXdgOutputV1.
 */
WaylandOutput *WaylandXdgOutputV1::output() const
{
    Q_D(const WaylandXdgOutputV1);
    return d->output;
}

/*!
 * \qmlproperty string XdgOutputV1::name
 *
 * This property holds the name of this output.
 *
 * The naming convention is compositor defined, but limited to alphanumeric
 * characters and dashes ("-").  Each name is unique and will also remain
 * consistent across sessions with the same hardware and software configuration.
 *
 * Examples of names include "HDMI-A-1", "WL-1", "X11-1" etc...
 * However don't assume the name reflects the underlying technology.
 *
 * Changing this property after initialization doesn't take effect.
 */
/*!
 * \property WaylandXdgOutputV1::name
 *
 * This property holds the name of this output.
 *
 * The naming convention is compositor defined, but limited to alphanumeric
 * characters and dashes ("-").  Each name is unique and will also remain
 * consistent across sessions with the same hardware and software configuration.
 *
 * Examples of names include "HDMI-A-1", "WL-1", "X11-1" etc...
 * However don't assume the name reflects the underlying technology.
 *
 * Changing this property after initialization doesn't take effect.
 */
QString WaylandXdgOutputV1::name() const
{
    Q_D(const WaylandXdgOutputV1);
    return d->name;
}

void WaylandXdgOutputV1::setName(const QString &name)
{
    Q_D(WaylandXdgOutputV1);

    if (d->name == name)
        return;

    // Can't change after clients bound to xdg-output
    if (d->initialized) {
        qCWarning(gLcAuroraCompositor, "WaylandXdgOutputV1::name cannot be changed after initialization");
        return;
    }

    d->name = name;
    emit nameChanged();
}

/*!
 *  \qmlproperty string XdgOutputV1::description
 *
 *  This property holds the description of this output.
 *
 *  No convention is defined for the description.
 *
 * Changing this property after initialization doesn't take effect.
 */
/*!
 * \property WaylandXdgOutputV1::description
 *
 *  This property holds the description of this output.
 *
 *  No convention is defined for the description.
 *
 * Changing this property after initialization doesn't take effect.
 */
QString WaylandXdgOutputV1::description() const
{
    Q_D(const WaylandXdgOutputV1);
    return d->description;
}

void WaylandXdgOutputV1::setDescription(const QString &description)
{
    Q_D(WaylandXdgOutputV1);

    if (d->description == description)
        return;

    // Can't change after clients bound to xdg-output
    if (d->initialized) {
        qCWarning(gLcAuroraCompositor, "WaylandXdgOutputV1::description cannot be changed after initialization");
        return;
    }

    d->description = description;
    emit descriptionChanged();
}

/*!
 * \qmlproperty point XdgOutputV1::logicalPosition
 *
 * This property holds the coordinates of the output within the global compositor space.
 *
 * The default value is 0,0.
 */
/*!
 * \property WaylandXdgOutputV1::logicalPosition
 *
 * This property holds the coordinates of the output within the global compositor space.
 *
 * The default value is 0,0.
 */
QPoint WaylandXdgOutputV1::logicalPosition() const
{
    Q_D(const WaylandXdgOutputV1);
    return d->logicalPos;
}

void WaylandXdgOutputV1::setLogicalPosition(const QPoint &position)
{
    Q_D(WaylandXdgOutputV1);

    if (d->logicalPos == position)
        return;

    d->logicalPos = position;
    if (d->initialized) {
        d->sendLogicalPosition(position);
        d->sendDone();
    }
    emit logicalPositionChanged();
    emit logicalGeometryChanged();
}

/*!
 * \qmlproperty size XdgOutputV1::logicalSize
 *
 * This property holds the size of the output in the global compositor space.
 *
 * The default value is -1,-1 which is invalid.
 *
 * Please remember that this is the logical size, not the physical size.
 * For example, for a WaylandOutput mode 3840x2160 and a scale factor 2:
 * \list
 * \li A compositor not scaling the surface buffers, will report a logical size of 3840x2160.
 * \li A compositor automatically scaling the surface buffers, will report a logical size of 1920x1080.
 * \li A compositor using a fractional scale of 1.5, will report a logical size of 2560x1620.
 * \endlist
 */
/*!
 * \property WaylandXdgOutputV1::logicalSize
 *
 * This property holds the size of the output in the global compositor space.
 *
 * The default value is -1,-1 which is invalid.
 *
 * Please remember that this is the logical size, not the physical size.
 * For example, for a WaylandOutput mode 3840x2160 and a scale factor 2:
 * \list
 * \li A compositor not scaling the surface buffers, will report a logical size of 3840x2160.
 * \li A compositor automatically scaling the surface buffers, will report a logical size of 1920x1080.
 * \li A compositor using a fractional scale of 1.5, will report a logical size of 2560x1620.
 * \endlist
 */
QSize WaylandXdgOutputV1::logicalSize() const
{
    Q_D(const WaylandXdgOutputV1);
    return d->logicalSize;
}

void WaylandXdgOutputV1::setLogicalSize(const QSize &size)
{
    Q_D(WaylandXdgOutputV1);

    if (d->logicalSize == size)
        return;

    d->logicalSize = size;
    if (d->initialized) {
        d->sendLogicalSize(size);
        d->sendDone();
    }
    emit logicalSizeChanged();
    emit logicalGeometryChanged();
}

/*!
 * \qmlproperty rect XdgOutputV1::logicalGeometry
 * \readonly
 *
 * This property holds the position and size of the output in the global compositor space.
 * It's the combination of the logical position and logical size.
 *
 * \sa XdgOutputV1::logicalPosition
 * \sa XdgOutputV1::logicalSize
 */
/*!
 * \property WaylandXdgOutputV1::logicalGeometry
 * \readonly
 *
 * This property holds the position and size of the output in the global compositor space.
 * It's the combination of the logical position and logical size.
 *
 * \sa WaylandXdgOutputV1::logicalPosition
 * \sa WaylandXdgOutputV1::logicalSize
 */
QRect WaylandXdgOutputV1::logicalGeometry() const
{
    Q_D(const WaylandXdgOutputV1);
    return QRect(d->logicalPos, d->logicalSize);
}

// WaylandXdgOutputV1Private

void WaylandXdgOutputV1Private::sendLogicalPosition(const QPoint &position)
{
    const auto values = resourceMap().values();
    for (auto *resource : values)
        send_logical_position(resource->handle, position.x(), position.y());
    needToSendDone = true;
}

void WaylandXdgOutputV1Private::sendLogicalSize(const QSize &size)
{
    const auto values = resourceMap().values();
    for (auto *resource : values)
        send_logical_size(resource->handle, size.width(), size.height());
    needToSendDone = true;
}

void WaylandXdgOutputV1Private::sendDone()
{
    if (needToSendDone) {
        const auto values = resourceMap().values();
        for (auto *resource : values) {
            if (resource->version() < 3)
                send_done(resource->handle);
        }
        needToSendDone = false;
    }
}

void WaylandXdgOutputV1Private::setManager(WaylandXdgOutputManagerV1 *_manager)
{
    Q_Q(WaylandXdgOutputV1);

    if (!_manager) {
        qCWarning(gLcAuroraCompositor,
                  "Cannot associate a null WaylandXdgOutputManagerV1 to WaylandXdgOutputV1 %p", this);
        return;
    }

    if (manager == _manager)
        return;

    if (manager) {
        qCWarning(gLcAuroraCompositor,
                  "Cannot associate a different WaylandXdgOutputManagerV1 to WaylandXdgOutputV1 %p "
                  "after initialization", this);
        return;
    }

    manager = _manager;
    emit q->managerChanged();
}

void WaylandXdgOutputV1Private::setOutput(WaylandOutput *_output)
{
    Q_Q(WaylandXdgOutputV1);

    if (!_output) {
        qCWarning(gLcAuroraCompositor,
                  "Cannot associate a null WaylandOutput to WaylandXdgOutputV1 %p", this);
        return;
    }

    if (output == _output)
        return;

    if (output) {
        qCWarning(gLcAuroraCompositor,
                  "Cannot associate a different WaylandOutput to WaylandXdgOutputV1 %p "
                  "after initialization", this);
        return;
    }

    // Assign output above manager, to make both values valid in handlers
    output = _output;

    if (!manager) {
        // Try to find the manager from the output parents
        for (auto *p = output->parent(); p != nullptr; p = p->parent()) {
            if (auto *m = qobject_cast<WaylandXdgOutputManagerV1 *>(p)) {
                manager = m;
                emit q->managerChanged();
                break;
            }
        }
    }

    emit q->outputChanged();

    // Register the output
    if (manager)
        WaylandXdgOutputManagerV1Private::get(manager)->registerXdgOutput(output, q);
}

void WaylandXdgOutputV1Private::zxdg_output_v1_bind_resource(Resource *resource)
{
    send_logical_position(resource->handle, logicalPos.x(), logicalPos.y());
    send_logical_size(resource->handle, logicalSize.width(), logicalSize.height());
    if (resource->version() >= ZXDG_OUTPUT_V1_NAME_SINCE_VERSION)
        send_name(resource->handle, name);
    if (resource->version() >= ZXDG_OUTPUT_V1_DESCRIPTION_SINCE_VERSION)
        send_description(resource->handle, description);
    send_done(resource->handle);

    initialized = true;
}

void WaylandXdgOutputV1Private::zxdg_output_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandxdgoutputv1.cpp"
