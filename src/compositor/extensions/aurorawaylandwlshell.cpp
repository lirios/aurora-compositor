// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandwlshell.h"
#include "aurorawaylandwlshell_p.h"

#if LIRI_FEATURE_aurora_compositor_quick
#include "aurorawaylandwlshellintegration_p.h"
#endif
#include <LiriAuroraCompositor/private/aurorawaylandutils_p.h>

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandView>
#include <LiriAuroraCompositor/WaylandOutput>
#include <LiriAuroraCompositor/WaylandClient>

#include <QtCore/QObject>
#include <QtCore/QDebug>

namespace Aurora {

namespace Compositor {

WaylandSurfaceRole WaylandWlShellSurfacePrivate::s_role("wl_shell_surface");

WaylandWlShellPrivate::WaylandWlShellPrivate()
{
}

void WaylandWlShellPrivate::shell_get_shell_surface(Resource *resource, uint32_t id, struct ::wl_resource *surface_res)
{
    Q_Q(WaylandWlShell);
    WaylandSurface *surface = WaylandSurface::fromResource(surface_res);

    WaylandResource shellSurfaceResource(wl_resource_create(resource->client(), &wl_shell_surface_interface,
                                                             wl_resource_get_version(resource->handle), id));

    // XXX FIXME
    // The role concept was formalized in wayland 1.7, so that release adds one error
    // code for each interface that implements a role, and we are supposed to pass here
    // the newly constructed resource and the correct error code so that if setting the
    // role fails, a proper error can be sent to the client.
    // However we're still using wayland 1.4, which doesn't have interface specific role
    // errors, so the best we can do is to use wl_display's object_id error.
    wl_resource *displayRes = wl_client_get_object(resource->client(), 1);
    if (!surface->setRole(WaylandWlShellSurface::role(), displayRes, WL_DISPLAY_ERROR_INVALID_OBJECT))
        return;

    emit q->wlShellSurfaceRequested(surface, shellSurfaceResource);

    WaylandWlShellSurface *shellSurface = WaylandWlShellSurface::fromResource(shellSurfaceResource.resource());
    if (!shellSurface) {
        // A WaylandWlShellSurface was not created in response to the wlShellSurfaceRequested
        // signal, so we create one as fallback here instead.
        shellSurface = new WaylandWlShellSurface(q, surface, shellSurfaceResource);
    }

    m_shellSurfaces.append(shellSurface);
    emit q->wlShellSurfaceCreated(shellSurface);
}

void WaylandWlShellPrivate::unregisterShellSurface(WaylandWlShellSurface *shellSurface)
{
    if (!m_shellSurfaces.removeOne(shellSurface))
        qWarning("Unexpected state. Can't find registered shell surface.");
}

WaylandWlShellSurfacePrivate::WaylandWlShellSurfacePrivate()
{
}

WaylandWlShellSurfacePrivate::~WaylandWlShellSurfacePrivate()
{
}

void WaylandWlShellSurfacePrivate::ping(uint32_t serial)
{
    m_pings.insert(serial);
    send_ping(serial);
}

void WaylandWlShellSurfacePrivate::setWindowType(Qt::WindowType windowType)
{
    if (m_windowType == windowType)
        return;
    m_windowType = windowType;

    Q_Q(WaylandWlShellSurface);
    emit q->windowTypeChanged();
}

void WaylandWlShellSurfacePrivate::shell_surface_destroy_resource(Resource *)
{
    Q_Q(WaylandWlShellSurface);

    delete q;
}

void WaylandWlShellSurfacePrivate::shell_surface_move(Resource *resource,
                struct wl_resource *input_device_super,
                uint32_t serial)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);

    Q_Q(WaylandWlShellSurface);
    WaylandSeat *input_device = WaylandSeat::fromSeatResource(input_device_super);
    emit q->startMove(input_device);
}

void WaylandWlShellSurfacePrivate::shell_surface_resize(Resource *resource,
                  struct wl_resource *input_device_super,
                  uint32_t serial,
                  uint32_t edges)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_Q(WaylandWlShellSurface);

    WaylandSeat *input_device = WaylandSeat::fromSeatResource(input_device_super);
    emit q->startResize(input_device, WaylandWlShellSurface::ResizeEdge(edges));
}

void WaylandWlShellSurfacePrivate::shell_surface_set_toplevel(Resource *resource)
{
    Q_UNUSED(resource);
    Q_Q(WaylandWlShellSurface);
    setWindowType(Qt::WindowType::Window);
    emit q->setDefaultToplevel();
}

void WaylandWlShellSurfacePrivate::shell_surface_set_transient(Resource *resource,
                      struct wl_resource *parent_surface_resource,
                      int x,
                      int y,
                      uint32_t flags)
{

    Q_UNUSED(resource);
    Q_Q(WaylandWlShellSurface);
    WaylandSurface *parent_surface = WaylandSurface::fromResource(parent_surface_resource);
    setWindowType(Qt::WindowType::SubWindow);
    emit q->setTransient(parent_surface, QPoint(x,y), flags & WL_SHELL_SURFACE_TRANSIENT_INACTIVE);
}

void WaylandWlShellSurfacePrivate::shell_surface_set_fullscreen(Resource *resource,
                       uint32_t method,
                       uint32_t framerate,
                       struct wl_resource *output_resource)
{
    Q_UNUSED(resource);
    Q_UNUSED(method);
    Q_UNUSED(framerate);
    Q_Q(WaylandWlShellSurface);
    WaylandOutput *output = output_resource
            ? WaylandOutput::fromResource(output_resource)
            : nullptr;
    setWindowType(Qt::WindowType::Window);
    emit q->setFullScreen(WaylandWlShellSurface::FullScreenMethod(method), framerate, output);
}

void WaylandWlShellSurfacePrivate::shell_surface_set_popup(Resource *resource, wl_resource *input_device, uint32_t serial, wl_resource *parent, int32_t x, int32_t y, uint32_t flags)
{
    Q_UNUSED(resource);
    Q_UNUSED(serial);
    Q_UNUSED(flags);
    Q_Q(WaylandWlShellSurface);
    WaylandSeat *input = WaylandSeat::fromSeatResource(input_device);
    WaylandSurface *parentSurface = WaylandSurface::fromResource(parent);
    setWindowType(Qt::WindowType::Popup);
    emit q->setPopup(input, parentSurface, QPoint(x,y));

}

void WaylandWlShellSurfacePrivate::shell_surface_set_maximized(Resource *resource,
                       struct wl_resource *output_resource)
{
    Q_UNUSED(resource);
    Q_Q(WaylandWlShellSurface);
    WaylandOutput *output = output_resource
            ? WaylandOutput::fromResource(output_resource)
            : nullptr;
    setWindowType(Qt::WindowType::Window);
    emit q->setMaximized(output);
}

void WaylandWlShellSurfacePrivate::shell_surface_pong(Resource *resource,
                        uint32_t serial)
{
    Q_UNUSED(resource);
    Q_Q(WaylandWlShellSurface);
    if (m_pings.remove(serial))
        emit q->pong();
    else
        qWarning("Received an unexpected pong!");
}

void WaylandWlShellSurfacePrivate::shell_surface_set_title(Resource *resource,
                             const QString &title)
{
    Q_UNUSED(resource);
    if (title == m_title)
        return;
    Q_Q(WaylandWlShellSurface);
    m_title = title;
    emit q->titleChanged();
}

void WaylandWlShellSurfacePrivate::shell_surface_set_class(Resource *resource,
                             const QString &className)
{
    Q_UNUSED(resource);
    if (className == m_className)
        return;
    Q_Q(WaylandWlShellSurface);
    m_className = className;
    emit q->classNameChanged();
}

/*!
 * \qmltype WlShell
 * \instantiates WaylandWlShell
 * \inqmlmodule Aurora.Compositor.WlShell
 * \since 5.8
 * \brief Provides an extension for desktop-style user interfaces.
 *
 * The WlShell extension provides a way to associate a ShellSurface
 * with a regular Wayland surface. Using the shell surface interface, the client
 * can request that the surface is resized, moved, and so on.
 *
 * WlShell corresponds to the Wayland interface \c wl_shell.
 *
 * To provide the functionality of the shell extension in a compositor, create
 * an instance of the WlShell component and add it to the list of extensions
 * supported by the compositor:
 *
 * \qml
 * import Aurora.Compositor.WlShell
 *
 * WaylandCompositor {
 *     WlShell {
 *         // ...
 *     }
 * }
 * \endqml
 */

/*!
 * \class WaylandWlShell
 * \inmodule AuroraCompositor
 * \since 5.8
 * \brief The WaylandWlShell class is an extension for desktop-style user interfaces.
 *
 * The WaylandWlShell extension provides a way to associate a WaylandWlShellSurface with
 * a regular Wayland surface. Using the shell surface interface, the client
 * can request that the surface is resized, moved, and so on.
 *
 * WlShell corresponds to the Wayland interface \c wl_shell.
 */

/*!
 * Constructs a WaylandWlShell object.
 */
WaylandWlShell::WaylandWlShell()
    : WaylandShellTemplate<WaylandWlShell>(*new WaylandWlShellPrivate())
{ }

/*!
 * Constructs a WaylandWlShell object for the provided \a compositor.
 */
WaylandWlShell::WaylandWlShell(WaylandCompositor *compositor)
    : WaylandShellTemplate<WaylandWlShell>(compositor, *new WaylandWlShellPrivate())
{ }


/*!
 * Initializes the WlShell extension.
 */
void WaylandWlShell::initialize()
{
    Q_D(WaylandWlShell);
    WaylandShellTemplate::initialize();
    WaylandCompositor *compositor = qobject_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find WaylandCompositor when initializing WaylandWlShell";
        return;
    }
    d->init(compositor->display(), 1);
}

QList<WaylandWlShellSurface *> WaylandWlShell::shellSurfaces() const
{
    Q_D(const WaylandWlShell);
    return d->m_shellSurfaces;
}

QList<WaylandWlShellSurface *> WaylandWlShell::shellSurfacesForClient(WaylandClient *client) const
{
    Q_D(const WaylandWlShell);
    QList<WaylandWlShellSurface *> surfsForClient;
    for (WaylandWlShellSurface *shellSurface : d->m_shellSurfaces) {
        if (shellSurface->surface() && shellSurface->surface()->client() == client)
            surfsForClient.append(shellSurface);
    }
    return surfsForClient;
}

QList<WaylandWlShellSurface *> WaylandWlShell::mappedPopups() const
{
    Q_D(const WaylandWlShell);
    QList<WaylandWlShellSurface *> popupSurfaces;
    for (WaylandWlShellSurface *shellSurface : d->m_shellSurfaces) {
        if (shellSurface->windowType() == Qt::WindowType::Popup
                && shellSurface->surface() && shellSurface->surface()->hasContent()) {
            popupSurfaces.append(shellSurface);
        }
    }
    return popupSurfaces;
}

WaylandClient *WaylandWlShell::popupClient() const
{
    Q_D(const WaylandWlShell);
    for (WaylandWlShellSurface *shellSurface : d->m_shellSurfaces) {
        if (shellSurface->windowType() == Qt::WindowType::Popup
                && shellSurface->surface() && shellSurface->surface()->hasContent()) {
            return shellSurface->surface()->client();
        }
    }
    return nullptr;
}

void WaylandWlShell::closeAllPopups()
{
    const auto mapped = mappedPopups();
    for (WaylandWlShellSurface *shellSurface : mapped)
        shellSurface->sendPopupDone();
}

/*!
 * Returns the Wayland interface for the WaylandWlShell.
 */
const struct wl_interface *WaylandWlShell::interface()
{
    return WaylandWlShellPrivate::interface();
}

/*!
 * \qmlsignal void WlShell::wlShellSurfaceRequested(WaylandSurface surface, WaylandResource resource)
 *
 * This signal is emitted when the client has requested a \c wl_shell_surface to be associated with
 * \a surface. The handler for this signal may create a shell surface for \a resource and initialize
 * it within the scope of the signal emission. Otherwise a WlShellSurface will be created
 * automatically.
 */

/*!
 * \fn void WaylandWlShell::wlShellSurfaceRequested(WaylandSurface *surface, const WaylandResource &resource)
 *
 * This signal is emitted when the client has requested a \c wl_shell_surface to be associated with
 * \a surface. The handler for this signal may create a shell surface for \a resource and initialize
 * it within the scope of the signal emission. Otherwise a WaylandWlShellSurface will be created
 * automatically.
 */

/*!
 * \qmlsignal void WlShell::wlShellSurfaceCreated(WlShellSurface shellSurface)
 *
 * This signal is emitted when the client has created a \c wl_shell_surface.
 * A common use case is to let the handler of this signal instantiate a ShellSurfaceItem or
 * WaylandQuickItem for displaying \a shellSurface in a QtQuick scene.
 */

/*!
 * \fn void WaylandWlShell::wlShellSurfaceCreated(WaylandWlShellSurface *shellSurface)
 *
 * This signal is emitted when the client has created a \c wl_shell_surface.
 * A common use case is to let the handler of this signal instantiate a WaylandShellSurfaceItem or
 * WaylandQuickItem for displaying \a shellSurface in a QtQuick scene.
 */

/*!
 * \internal
 */
QByteArray WaylandWlShell::interfaceName()
{
    return WaylandWlShellPrivate::interfaceName();
}

/*!
 * \qmltype WlShellSurface
 * \instantiates WaylandWlShellSurface
 * \inqmlmodule Aurora.Compositor.WlShell
 * \since 5.8
 * \brief Provides a \c wl_shell_surface that offers desktop-style compositor-specific features to a surface.
 *
 * This type is part of the \l{WlShell} extension and provides a way to extend
 * the functionality of an existing WaylandSurface with features specific to desktop-style
 * compositors, such as resizing and moving the surface.
 *
 * It corresponds to the Wayland interface \c wl_shell_surface.
 */

/*!
 * \class WaylandWlShellSurface
 * \inmodule AuroraCompositor
 * \since 5.8
 * \brief The WaylandWlShellSurface class provides desktop-style compositor-specific features to a surface.
 *
 * This class is part of the WaylandWlShell extension and provides a way to extend
 * the functionality of an existing WaylandSurface with features specific to desktop-style
 * compositors, such as resizing and moving the surface.
 *
 * It corresponds to the Wayland interface \c wl_shell_surface.
 */

/*!
 * Constructs a WaylandWlShellSurface.
 */
WaylandWlShellSurface::WaylandWlShellSurface()
    : WaylandShellSurfaceTemplate<WaylandWlShellSurface>(*new WaylandWlShellSurfacePrivate)
{
}

/*!
 * Constructs a WaylandWlShellSurface for \a surface and initializes it with the given \a shell and resource \a res.
 */
WaylandWlShellSurface::WaylandWlShellSurface(WaylandWlShell *shell, WaylandSurface *surface, const WaylandResource &res)
    : WaylandShellSurfaceTemplate<WaylandWlShellSurface>(*new WaylandWlShellSurfacePrivate)
{
    initialize(shell, surface, res);
}

WaylandWlShellSurface::~WaylandWlShellSurface()
{
    Q_D(WaylandWlShellSurface);
    if (d->m_shell)
        WaylandWlShellPrivate::get(d->m_shell)->unregisterShellSurface(this);
}

/*!
 * \qmlmethod void WlShellSurface::initialize(WlShell shell, WaylandSurface surface, WaylandResource resource)
 *
 * Initializes the WlShellSurface and associates it with the given \a shell, \a surface, and \a resource.
 */

/*!
 * Initializes the WaylandWlShellSurface and associates it with the given \a shell, \a surface, and \a resource.
 */
void WaylandWlShellSurface::initialize(WaylandWlShell *shell, WaylandSurface *surface, const WaylandResource &resource)
{
    Q_D(WaylandWlShellSurface);
    d->m_shell = shell;
    d->m_surface = surface;
    d->init(resource.resource());
    setExtensionContainer(surface);
    emit surfaceChanged();
    emit shellChanged();
    WaylandCompositorExtension::initialize();
}

/*!
 * \internal
 */
void WaylandWlShellSurface::initialize()
{
    WaylandCompositorExtension::initialize();
}

const struct wl_interface *WaylandWlShellSurface::interface()
{
    return WaylandWlShellSurfacePrivate::interface();
}

/*!
 * \internal
 */
QByteArray WaylandWlShellSurface::interfaceName()
{
    return WaylandWlShellSurfacePrivate::interfaceName();
}

QSize WaylandWlShellSurface::sizeForResize(const QSizeF &size, const QPointF &delta, WaylandWlShellSurface::ResizeEdge edge)
{
    qreal width = size.width();
    qreal height = size.height();
    if (edge & LeftEdge)
        width -= delta.x();
    else if (edge & RightEdge)
        width += delta.x();

    if (edge & TopEdge)
        height -= delta.y();
    else if (edge & BottomEdge)
        height += delta.y();

    QSizeF newSize(qMax(width, 1.0), qMax(height, 1.0));
    return newSize.toSize();
}

/*!
 * \enum WaylandWlShellSurface::ResizeEdge
 *
 * This enum type provides a way to specify an edge or corner of
 * the surface.
 *
 * \value NoneEdge No edge.
 * \value TopEdge The top edge.
 * \value BottomEdge The bottom edge.
 * \value LeftEdge The left edge.
 * \value TopLeftEdge The top left corner.
 * \value BottomLeftEdge The bottom left corner.
 * \value RightEdge The right edge.
 * \value TopRightEdge The top right corner.
 * \value BottomRightEdge The bottom right corner.
 */

/*!
 * \qmlmethod void WlShellSurface::sendConfigure(size s, enum edges)
 *
 * Sends a configure event to the client, suggesting that it resize its surface to
 * the provided size \a s. The \a edges provide a hint about how the surface
 * was resized.
 */

/*!
 * Sends a configure event to the client, suggesting that it resize its surface to
 * the provided \a size. The \a edges provide a hint about how the surface
 * was resized.
 */
void WaylandWlShellSurface::sendConfigure(const QSize &size, ResizeEdge edges)
{
    Q_D(WaylandWlShellSurface);
    if (!size.isValid()) {
        qWarning() << "Can't configure wl_shell_surface with an invalid size" << size;
        return;
    }
    d->send_configure(edges, size.width(), size.height());
}

/*!
 * \qmlmethod void WlShellSurface::sendPopupDone()
 *
 * Sends a popup_done event to the client to indicate that the user has clicked
 * somewhere outside the client's surfaces.
 */

/*!
 * Sends a popup_done event to the client to indicate that the user has clicked
 * somewhere outside the client's surfaces.
 */
void WaylandWlShellSurface::sendPopupDone()
{
    Q_D(WaylandWlShellSurface);
    d->send_popup_done();
}

#if LIRI_FEATURE_aurora_compositor_quick
WaylandQuickShellIntegration *WaylandWlShellSurface::createIntegration(WaylandQuickShellSurfaceItem *item)
{
    return new Internal::WlShellIntegration(item);
}
#endif

/*!
 * \qmlproperty WaylandSurface WlShellSurface::surface
 *
 * This property holds the \c wl_surface associated with this WlShellSurface.
 */

/*!
 * \property WaylandWlShellSurface::surface
 *
 * This property holds the surface associated with this WaylandWlShellSurface.
 */
WaylandSurface *WaylandWlShellSurface::surface() const
{
    Q_D(const WaylandWlShellSurface);
    return d->m_surface;
}

/*!
 * \qmlproperty WlShell WlShellSurface::shell
 *
 * This property holds the shell associated with this WlShellSurface.
 */

/*!
 * \property WaylandWlShellSurface::shell
 *
 * This property holds the shell associated with this WaylandWlShellSurface.
 */
WaylandWlShell *WaylandWlShellSurface::shell() const
{
    Q_D(const WaylandWlShellSurface);
    return d->m_shell;
}

/*!
 * \qmlproperty enum WlShellSurface::windowType
 *
 * This property holds the window type of the WlShellSurface.
 */

Qt::WindowType WaylandWlShellSurface::windowType() const
{
    Q_D(const WaylandWlShellSurface);
    return d->m_windowType;
}

/*!
 * \qmlproperty string WlShellSurface::title
 *
 * This property holds the title of the WlShellSurface.
 */

/*!
 * \property WaylandWlShellSurface::title
 *
 * This property holds the title of the WaylandWlShellSurface.
 */
QString WaylandWlShellSurface::title() const
{
    Q_D(const WaylandWlShellSurface);
    return d->m_title;
}

/*!
 * \qmlproperty string WlShellSurface::className
 *
 * This property holds the class name of the WlShellSurface.
 */

/*!
 * \property WaylandWlShellSurface::className
 *
 * This property holds the class name of the WaylandWlShellSurface.
 */
QString WaylandWlShellSurface::className() const
{
    Q_D(const WaylandWlShellSurface);
    return d->m_className;
}

WaylandSurfaceRole *WaylandWlShellSurface::role()
{
    return &WaylandWlShellSurfacePrivate::s_role;
}

/*!
 * \qmlmethod void WlShellSurface::ping()
 *
 * Sends a ping event to the client. If the client replies to the event the pong
 * signal will be emitted.
 */

/*!
 * Sends a ping event to the client. If the client replies to the event the pong
 * signal will be emitted.
 */
void WaylandWlShellSurface::ping()
{
    Q_D(WaylandWlShellSurface);
    uint32_t serial = d->m_surface->compositor()->nextSerial();
    d->ping(serial);
}

/*!
 * Returns the WaylandWlShellSurface object associated with the given \a resource, or null if no such object exists.
 */
WaylandWlShellSurface *WaylandWlShellSurface::fromResource(wl_resource *resource)
{
    if (auto p = Internal::fromResource<WaylandWlShellSurfacePrivate *>(resource))
        return p->q_func();
    return nullptr;
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandwlshell.cpp"
