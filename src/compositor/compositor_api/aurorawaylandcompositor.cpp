/****************************************************************************
**
** Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2020 The Qt Company Ltd.
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

#include "qtwaylandcompositorglobal_p.h"
#include "aurorawaylandcompositor.h"
#include "aurorawaylandcompositor_p.h"

#include <LiriAuroraCompositor/aurorawaylandclient.h>
#include <LiriAuroraCompositor/aurorawaylandseat.h>
#include <LiriAuroraCompositor/aurorawaylandoutput.h>
#include <LiriAuroraCompositor/aurorawaylandview.h>
#include <LiriAuroraCompositor/aurorawaylandclient.h>
#include <LiriAuroraCompositor/aurorawaylandkeyboard.h>
#include <LiriAuroraCompositor/aurorawaylandpointer.h>
#include <LiriAuroraCompositor/aurorawaylandtouch.h>
#include <LiriAuroraCompositor/aurorawaylandsurfacegrabber.h>

#include <LiriAuroraCompositor/private/aurorawaylandkeyboard_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandsurface_p.h>

#if LIRI_FEATURE_aurora_datadevice
#include "wayland_wrapper/aurorawldatadevice_p.h"
#include "wayland_wrapper/aurorawldatadevicemanager_p.h"
#endif
#include "wayland_wrapper/aurorawlbuffermanager_p.h"

#include "hardware_integration/aurorawlclientbufferintegration_p.h"
#include "hardware_integration/aurorawlclientbufferintegrationfactory_p.h"
#include "hardware_integration/aurorawlserverbufferintegration_p.h"
#include "hardware_integration/aurorawlserverbufferintegrationfactory_p.h"

#if QT_CONFIG(opengl)
#include "hardware_integration/aurorawlhwintegration_p.h"
#endif

#include "extensions/aurorawaylandqtwindowmanager.h"

#include "aurorawaylandsharedmemoryformathelper_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>
#include <QtCore/QSocketNotifier>

#include <QtGui/QDesktopServices>
#include <QtGui/QScreen>

#include <QtGui/qpa/qwindowsysteminterface_p.h>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtGui/private/qguiapplication_p.h>

#if QT_CONFIG(opengl)
#   include <QOpenGLTextureBlitter>
#   include <QOpenGLTexture>
#   include <QOpenGLContext>
#   include <QOpenGLFramebufferObject>
#   include <QMatrix4x4>
#endif

namespace Aurora {

namespace Compositor {

Q_LOGGING_CATEGORY(qLcWaylandCompositor, "qt.waylandcompositor")
Q_LOGGING_CATEGORY(qLcWaylandCompositorHardwareIntegration, "qt.waylandcompositor.hardwareintegration")
Q_LOGGING_CATEGORY(qLcWaylandCompositorInputMethods, "qt.waylandcompositor.inputmethods")
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
Q_LOGGING_CATEGORY(qLcWaylandCompositorTextInput, "qt.waylandcompositor.textinput")
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP

namespace Internal {

class WindowSystemEventHandler : public QWindowSystemEventHandler
{
public:
    WindowSystemEventHandler(WaylandCompositor *c) : compositor(c) {}
    bool sendEvent(QWindowSystemInterfacePrivate::WindowSystemEvent *e) override
    {
        if (e->type == QWindowSystemInterfacePrivate::Key) {
            QWindowSystemInterfacePrivate::KeyEvent *keyEvent = static_cast<QWindowSystemInterfacePrivate::KeyEvent *>(e);
            handleKeyEvent(keyEvent);
        } else {
            QWindowSystemEventHandler::sendEvent(e);
        }
        return true;
    }

    void handleKeyEvent(QWindowSystemInterfacePrivate::KeyEvent *ke)
    {
        auto *seat = compositor->defaultSeat();
        if (!seat)
            return;

        WaylandKeyboardPrivate *keyb = WaylandKeyboardPrivate::get(seat->keyboard());

#if defined(Q_OS_QNX)
        // The QNX platform plugin delivers scan codes that haven't been adjusted to be
        // xkbcommon compatible. xkbcommon requires that the scan codes be bumped up by
        // 8 because that's how evdev/XKB deliver scan codes. You might think that it
        // would've been better to remove this (odd) requirement from xkbcommon on QNX
        // but it turns out that conforming to it has much less impact.
        static int offset = QGuiApplication::platformName() == QStringLiteral("qnx") ? 8 : 0;
        ke->nativeScanCode += offset;
#endif
        uint32_t code = ke->nativeScanCode;
        bool isDown = ke->keyType == QEvent::KeyPress;

#if LIRI_FEATURE_aurora_xkbcommon
        xkb_state *xkbState = keyb->xkbState();
        Qt::KeyboardModifiers modifiers = QXkbCommon::modifiers(xkbState);

        const xkb_keysym_t sym = xkb_state_key_get_one_sym(xkbState, code);
        int qtkey = QXkbCommon::keysymToQtKey(sym, modifiers, xkbState, code);
        QString text = QXkbCommon::lookupString(xkbState, code);

        ke->key = qtkey;
        ke->modifiers = modifiers;
        ke->nativeVirtualKey = sym;
        ke->nativeModifiers = keyb->xkbModsMask();
        ke->unicode = text;
#endif
        if (!ke->repeat)
            keyb->keyEvent(code, isDown ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED);

        QWindowSystemEventHandler::sendEvent(ke);

        if (!ke->repeat) {
            keyb->maybeUpdateKeymap();
            keyb->updateModifierState(code, isDown ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED);
        }
    }

    WaylandCompositor *compositor = nullptr;
};

} // namespace

WaylandCompositorPrivate::WaylandCompositorPrivate(WaylandCompositor *compositor)
{
    if (QGuiApplication::platformNativeInterface())
        display = static_cast<wl_display*>(QGuiApplication::platformNativeInterface()->nativeResourceForIntegration("server_wl_display"));

    if (!display) {
        display = wl_display_create();
        ownsDisplay = true;
    }

    eventHandler.reset(new Internal::WindowSystemEventHandler(compositor));
    timer.start();

    QWindowSystemInterfacePrivate::installWindowSystemEventHandler(eventHandler.data());

#if LIRI_FEATURE_aurora_xkbcommon
    mXkbContext.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));
    if (!mXkbContext) {
        qWarning("Failed to create a XKB context: keymap will not be supported");
        return;
    }
#endif
}

void WaylandCompositorPrivate::init()
{
    Q_Q(WaylandCompositor);
    QStringList arguments = QCoreApplication::instance()->arguments();

    if (socket_name.isEmpty()) {
        const int socketArg = arguments.indexOf(QLatin1String("--wayland-socket-name"));
        if (socketArg != -1 && socketArg + 1 < arguments.size())
            socket_name = arguments.at(socketArg + 1).toLocal8Bit();
    }
    wl_compositor::init(display, 4);
    wl_subcompositor::init(display, 1);

#if LIRI_FEATURE_aurora_datadevice
    data_device_manager =  new Internal::DataDeviceManager(q);
#endif
    buffer_manager = new Internal::BufferManager(q);

    wl_display_init_shm(display);

    for (WaylandCompositor::ShmFormat format : shmFormats)
        wl_display_add_shm_format(display, wl_shm_format(format));

    if (!socket_name.isEmpty()) {
        if (wl_display_add_socket(display, socket_name.constData()))
            qFatal("Fatal: Failed to open server socket: \"%s\". XDG_RUNTIME_DIR is: \"%s\"\n", socket_name.constData(), getenv("XDG_RUNTIME_DIR"));
    } else {
        const char *autoSocketName = wl_display_add_socket_auto(display);
        if (!autoSocketName)
            qFatal("Fatal: Failed to open default server socket. XDG_RUNTIME_DIR is: \"%s\"\n", getenv("XDG_RUNTIME_DIR"));
        socket_name = autoSocketName;
        emit q->socketNameChanged(socket_name);
    }

    connectToExternalSockets();

    loop = wl_display_get_event_loop(display);

    int fd = wl_event_loop_get_fd(loop);

    QSocketNotifier *sockNot = new QSocketNotifier(fd, QSocketNotifier::Read, q);
    QObject::connect(sockNot, SIGNAL(activated(QSocketDescriptor)), q, SLOT(processWaylandEvents()));

    QAbstractEventDispatcher *dispatcher = QGuiApplicationPrivate::eventDispatcher;
    QObject::connect(dispatcher, SIGNAL(aboutToBlock()), q, SLOT(processWaylandEvents()));

    QObject::connect(static_cast<QGuiApplication *>(QGuiApplication::instance()),
                     &QGuiApplication::applicationStateChanged,
                     q,
                     &WaylandCompositor::applicationStateChanged);

    initializeHardwareIntegration();
    initializeSeats();

    initialized = true;

    for (const QPointer<QObject> &object : qExchange(polish_objects, {})) {
        if (object) {
            QEvent polishEvent(QEvent::Polish);
            QCoreApplication::sendEvent(object.data(), &polishEvent);
        }
    }

    emit q->createdChanged();
}

WaylandCompositorPrivate::~WaylandCompositorPrivate()
{
    // Take copies, since the lists will get modified as elements are deleted
    const auto clientsToDelete = clients;
    qDeleteAll(clientsToDelete);

    const auto outputsToDelete = outputs;
    qDeleteAll(outputsToDelete);

#if LIRI_FEATURE_aurora_datadevice
    delete data_device_manager;
#endif

    // Some client buffer integrations need to clean up before the destroying the wl_display
    qDeleteAll(client_buffer_integrations);

    if (ownsDisplay)
        wl_display_destroy(display);
}

void WaylandCompositorPrivate::preInit()
{
    Q_Q(WaylandCompositor);

    if (preInitialized)
        return;

    if (seats.empty())
        seats.append(q->createSeat());

    preInitialized = true;
}

void WaylandCompositorPrivate::destroySurface(WaylandSurface *surface)
{
    Q_Q(WaylandCompositor);
    emit q->surfaceAboutToBeDestroyed(surface);

    delete surface;
}

void WaylandCompositorPrivate::unregisterSurface(WaylandSurface *surface)
{
    if (!all_surfaces.removeOne(surface))
        qWarning("%s Unexpected state. Cant find registered surface\n", Q_FUNC_INFO);
}

void WaylandCompositorPrivate::feedRetainedSelectionData(QMimeData *data)
{
    Q_Q(WaylandCompositor);
    if (retainSelection)
        q->retainedSelectionReceived(data);
}

void WaylandCompositorPrivate::addPolishObject(QObject *object)
{
    if (initialized) {
        QCoreApplication::postEvent(object, new QEvent(QEvent::Polish));
    } else {
        polish_objects.push_back(object);
    }
}

void WaylandCompositorPrivate::connectToExternalSockets()
{
    // Clear out any backlog of user-supplied external socket descriptors
    for (int fd : qAsConst(externally_added_socket_fds)) {
        if (wl_display_add_socket_fd(display, fd) != 0)
            qWarning() << "Failed to integrate user-supplied socket fd into the Wayland event loop";
    }
    externally_added_socket_fds.clear();
}

void WaylandCompositorPrivate::compositor_create_surface(wl_compositor::Resource *resource, uint32_t id)
{
    Q_Q(WaylandCompositor);
    WaylandClient *client = WaylandClient::fromWlClient(q, resource->client());
    emit q->surfaceRequested(client, id, resource->version());
#ifndef QT_NO_DEBUG
    Q_ASSERT_X(!WaylandSurfacePrivate::hasUninitializedSurface(), "WaylandCompositor", QStringLiteral("Found uninitialized WaylandSurface after emitting WaylandCompositor::createSurface for id %1. All surfaces has to be initialized immediately after creation. See WaylandSurface::initialize.").arg(id).toLocal8Bit().constData());
#endif
    struct wl_resource *surfResource = wl_client_get_object(client->client(), id);

    WaylandSurface *surface = nullptr;
    if (surfResource) {
        surface = WaylandSurface::fromResource(surfResource);
    } else {
        surface = createDefaultSurface();
        surface->initialize(q, client, id, resource->version());
    }
    Q_ASSERT(surface);
    all_surfaces.append(surface);
    emit q->surfaceCreated(surface);
}

void WaylandCompositorPrivate::compositor_create_region(wl_compositor::Resource *resource, uint32_t id)
{
    new Internal::Region(resource->client(), id);
}

void WaylandCompositorPrivate::subcompositor_get_subsurface(wl_subcompositor::Resource *resource, uint32_t id, wl_resource *surface, wl_resource *parent)
{
    Q_Q(WaylandCompositor);
    WaylandSurface *childSurface = WaylandSurface::fromResource(surface);
    WaylandSurface *parentSurface = WaylandSurface::fromResource(parent);
    WaylandSurfacePrivate::get(childSurface)->initSubsurface(parentSurface, resource->client(), id, 1);
    WaylandSurfacePrivate::get(parentSurface)->subsurfaceChildren.append(childSurface);
    emit q->subsurfaceChanged(childSurface, parentSurface);
}

/*!
  \internal
  Used to create a fallback WaylandSurface when no surface was
  created by emitting the WaylandCompositor::createSurface signal.
*/
WaylandSurface *WaylandCompositorPrivate::createDefaultSurface()
{
    return new WaylandSurface();
}

class SharedMemoryClientBufferIntegration : public Internal::ClientBufferIntegration
{
public:
    void initializeHardware(wl_display *display) override;
    Internal::ClientBuffer *createBufferFor(wl_resource *buffer) override;
};

void SharedMemoryClientBufferIntegration::initializeHardware(wl_display *)
{
}

Internal::ClientBuffer *SharedMemoryClientBufferIntegration::createBufferFor(wl_resource *buffer)
{
    if (wl_shm_buffer_get(buffer))
        return new Internal::SharedMemoryBuffer(buffer);
    return nullptr;
}

void WaylandCompositorPrivate::initializeHardwareIntegration()
{
    client_buffer_integrations.prepend(new SharedMemoryClientBufferIntegration); // TODO: clean up the opengl dependency

#if QT_CONFIG(opengl)
    Q_Q(WaylandCompositor);
    if (use_hw_integration_extension)
        hw_integration.reset(new Internal::HardwareIntegration(q));

    loadClientBufferIntegration();
    loadServerBufferIntegration();

    for (auto *integration : qAsConst(client_buffer_integrations))
        integration->initializeHardware(display);
#endif
}

void WaylandCompositorPrivate::initializeSeats()
{
    for (WaylandSeat *seat : qAsConst(seats))
        seat->initialize();
}

void WaylandCompositorPrivate::loadClientBufferIntegration()
{
#if QT_CONFIG(opengl)
    Q_Q(WaylandCompositor);
    QStringList keys = Internal::ClientBufferIntegrationFactory::keys();
    QStringList targetKeys;
    QByteArray clientBufferIntegration = qgetenv("QT_WAYLAND_HARDWARE_INTEGRATION");
    if (clientBufferIntegration.isEmpty())
        clientBufferIntegration = qgetenv("QT_WAYLAND_CLIENT_BUFFER_INTEGRATION");

    for (auto b : clientBufferIntegration.split(';')) {
        QString s = QString::fromLocal8Bit(b);
        if (keys.contains(s))
            targetKeys.append(s);
    }

    if (targetKeys.isEmpty()) {
        if (keys.contains(QString::fromLatin1("wayland-egl"))) {
            targetKeys.append(QString::fromLatin1("wayland-egl"));
        } else if (!keys.isEmpty()) {
            targetKeys.append(keys.first());
        }
    }

    QString hwIntegrationName;

    for (auto targetKey : qAsConst(targetKeys)) {
        auto *integration = Internal::ClientBufferIntegrationFactory::create(targetKey, QStringList());
        if (integration) {
            integration->setCompositor(q);
            client_buffer_integrations.append(integration);
            if (hwIntegrationName.isEmpty())
                hwIntegrationName = targetKey;
        }
    }

    if (hw_integration && !hwIntegrationName.isEmpty())
        hw_integration->setClientBufferIntegrationName(hwIntegrationName);

#endif
}

void WaylandCompositorPrivate::loadServerBufferIntegration()
{
#if QT_CONFIG(opengl)
    Q_Q(WaylandCompositor);
    QStringList keys = Internal::ServerBufferIntegrationFactory::keys();
    QString targetKey;
    QByteArray serverBufferIntegration = qgetenv("QT_WAYLAND_SERVER_BUFFER_INTEGRATION");
    if (keys.contains(QString::fromLocal8Bit(serverBufferIntegration.constData()))) {
        targetKey = QString::fromLocal8Bit(serverBufferIntegration.constData());
    }
    if (!targetKey.isEmpty()) {
        server_buffer_integration.reset(Internal::ServerBufferIntegrationFactory::create(targetKey, QStringList()));
        if (server_buffer_integration) {
            qCDebug(qLcWaylandCompositorHardwareIntegration)
                    << "Loaded server buffer integration:" << targetKey;
            if (!server_buffer_integration->initializeHardware(q)) {
                qCWarning(qLcWaylandCompositorHardwareIntegration)
                        << "Failed to initialize hardware for server buffer integration:" << targetKey;
                server_buffer_integration.reset();
            }
        } else {
            qCWarning(qLcWaylandCompositorHardwareIntegration)
                    << "Failed to load server buffer integration:" << targetKey;
        }
    }

    if (server_buffer_integration && hw_integration)
        hw_integration->setServerBufferIntegrationName(targetKey);
#endif
}

WaylandSeat *WaylandCompositorPrivate::seatFor(QInputEvent *inputEvent)
{
    WaylandSeat *dev = nullptr;
    for (int i = 0; i < seats.size(); i++) {
        WaylandSeat *candidate = seats.at(i);
        if (candidate->isOwner(inputEvent)) {
            dev = candidate;
            break;
        }
    }
    return dev;
}

/*!
  \qmltype WaylandCompositor
  \instantiates WaylandCompositor
  \inqmlmodule Aurora.Compositor
  \since 5.8
  \brief Manages the Wayland display server.

  The WaylandCompositor manages the connections to the clients, as well as the different
  \l{WaylandOutput}{outputs} and \l{WaylandSeat}{seats}.

  Normally, a compositor application will have a single WaylandCompositor
  instance, which can have several outputs as children. When a client
  requests the compositor to create a surface, the request is handled by
  the onSurfaceRequested handler.

  Extensions that are supported by the compositor should be instantiated and added to the
  extensions property.
*/


/*!
   \class WaylandCompositor
   \inmodule QtWaylandCompositor
   \since 5.8
   \brief The WaylandCompositor class manages the Wayland display server.

   The WaylandCompositor manages the connections to the clients, as well as the different \l{WaylandOutput}{outputs}
   and \l{WaylandSeat}{seats}.

   Normally, a compositor application will have a single WaylandCompositor
   instance, which can have several outputs as children.
*/

/*!
  \qmlsignal void QtWaylandCompositor::WaylandCompositor::surfaceRequested(WaylandClient client, int id, int version)

  This signal is emitted when a \a client has created a surface with id \a id.
  The interface \a version is also available.

  The slot connecting to this signal may create and initialize a WaylandSurface
  instance in the scope of the slot. Otherwise a default surface is created.
*/

/*!
  \fn void WaylandCompositor::surfaceRequested(WaylandClient *client, uint id, int version)

  This signal is emitted when a \a client has created a surface with id \a id.
  The interface \a version is also available.

  The slot connecting to this signal may create and initialize a WaylandSurface
  instance in the scope of the slot. Otherwise a default surface is created.

  Connections to this signal must be of Qt::DirectConnection connection type.
*/

/*!
  \qmlsignal void QtWaylandCompositor::WaylandCompositor::surfaceCreated(WaylandSurface surface)

  This signal is emitted when a new WaylandSurface instance \a surface has been created.
*/

/*!
  \fn void WaylandCompositor::surfaceCreated(WaylandSurface *surface)

  This signal is emitted when a new WaylandSurface instance \a surface has been created.
*/

/*!
 * Constructs a WaylandCompositor with the given \a parent.
 */
WaylandCompositor::WaylandCompositor(QObject *parent)
    : WaylandObject(*new WaylandCompositorPrivate(this), parent)
{
}

/*!
 * \internal
 * Constructs a WaylandCompositor with the private object \a dptr and \a parent.
 */
WaylandCompositor::WaylandCompositor(WaylandCompositorPrivate &dptr, QObject *parent)
    : WaylandObject(dptr, parent)
{
}

/*!
 * Destroys the WaylandCompositor
 */
WaylandCompositor::~WaylandCompositor()
{
}

/*!
 * Initializes the WaylandCompositor.
 * If you override this function in your subclass, be sure to call the base class implementation.
 */
void WaylandCompositor::create()
{
    Q_D(WaylandCompositor);
    d->preInit();
    d->init();
}

/*!
 * \qmlproperty bool QtWaylandCompositor::WaylandCompositor::created
 *
 * This property is true if WaylandCompositor has been initialized,
 * otherwise it's false.
 */

/*!
 * \property WaylandCompositor::created
 *
 * This property is true if WaylandCompositor has been initialized,
 * otherwise it's false.
 */
bool WaylandCompositor::isCreated() const
{
    Q_D(const WaylandCompositor);
    return d->initialized;
}

/*!
 * \qmlproperty string QtWaylandCompositor::WaylandCompositor::socketName
 *
 * This property holds the socket name used by WaylandCompositor to communicate with
 * clients. It must be set before the component is completed.
 *
 * If the socketName is empty (the default), the contents of the start argument
 * \c --wayland-socket-name are used instead. If the argument is not set, the
 * compositor tries to find a socket name, which is \c{wayland-0} by default.
 */

/*!
 * \property WaylandCompositor::socketName
 *
 * This property holds the socket name used by WaylandCompositor to communicate with
 * clients. This must be set before the WaylandCompositor is \l{create()}{created}.
 *
 * If the socketName is empty (the default), the contents of the start argument
 * \c --wayland-socket-name are used instead. If the argument is not set, the
 * compositor tries to find a socket name, which is \c{wayland-0} by default.
 */
void WaylandCompositor::setSocketName(const QByteArray &name)
{
    Q_D(WaylandCompositor);

    if (d->socket_name == name)
        return;

    if (d->initialized) {
        qWarning("%s: Changing socket name after initializing the compositor is not supported.\n", Q_FUNC_INFO);
        return;
    }

    d->socket_name = name;
    emit socketNameChanged(name);
}

QByteArray WaylandCompositor::socketName() const
{
    Q_D(const WaylandCompositor);
    return d->socket_name;
}

/*!
 * \qmlmethod QtWaylandCompositor::WaylandCompositor::addSocketDescriptor(fd)
 * \since 5.12
 *
 * Listen for client connections on a file descriptor, \a fd, referring to a
 * server socket already bound and listening.
 *
 * Does not take ownership of the file descriptor; it must be closed
 * explicitly if needed.
 *
 * \note This method is only available with libwayland 1.10.0 or
 * newer. If built against an earlier libwayland runtime, this
 * method is a noop.
 */

/*!
 * Listen for client connections on a file descriptor, \a fd, referring to a
 * server socket already bound and listening.
 *
 * Does not take ownership of the file descriptor; it must be closed
 * explicitly if needed.
 *
 * \note This method is only available with libwayland 1.10.0 or
 * newer. If built against an earlier libwayland runtime, this
 * method is a noop.
 *
 * \since 5.12
 */
void WaylandCompositor::addSocketDescriptor(int fd)
{
    Q_D(WaylandCompositor);
    d->externally_added_socket_fds.append(fd);
    if (isCreated())
        d->connectToExternalSockets();
}

/*!
 * \internal
 */
struct wl_display *WaylandCompositor::display() const
{
    Q_D(const WaylandCompositor);
    return d->display;
}

/*!
 * \internal
 */
uint32_t WaylandCompositor::nextSerial()
{
    Q_D(WaylandCompositor);
    return wl_display_next_serial(d->display);
}

/*!
 * \internal
 */
QList<WaylandClient *>WaylandCompositor::clients() const
{
    Q_D(const WaylandCompositor);
    return d->clients;
}

/*!
 * \qmlmethod QtWaylandCompositor::WaylandCompositor::destroyClientForSurface(surface)
 *
 * Destroys the client for the WaylandSurface \a surface.
 */

/*!
 * Destroys the client for the \a surface.
 */
void WaylandCompositor::destroyClientForSurface(WaylandSurface *surface)
{
    destroyClient(surface->client());
}

/*!
 * \qmlmethod QtWaylandCompositor::WaylandCompositor::destroyClient(client)
 *
 * Destroys the given WaylandClient \a client.
 */

/*!
 * Destroys the \a client.
 */
void WaylandCompositor::destroyClient(WaylandClient *client)
{
    if (!client)
        return;

    WaylandQtWindowManager *wmExtension = WaylandQtWindowManager::findIn(this);
    if (wmExtension)
        wmExtension->sendQuitMessage(client);

    wl_client_destroy(client->client());
}

/*!
 * \internal
 */
QList<WaylandSurface *> WaylandCompositor::surfacesForClient(WaylandClient* client) const
{
    Q_D(const WaylandCompositor);
    QList<WaylandSurface *> surfs;
    for (WaylandSurface *surface : d->all_surfaces) {
        if (surface->client() == client)
            surfs.append(surface);
    }
    return surfs;
}

/*!
 * \internal
 */
QList<WaylandSurface *> WaylandCompositor::surfaces() const
{
    Q_D(const WaylandCompositor);
    return d->all_surfaces;
}

/*!
 * Returns the WaylandOutput that is connected to the given \a window.
 */
WaylandOutput *WaylandCompositor::outputFor(QWindow *window) const
{
    Q_D(const WaylandCompositor);
    for (WaylandOutput *output : d->outputs) {
        if (output->window() == window)
            return output;
    }

    return nullptr;
}

/*!
 * \qmlproperty WaylandOutput QtWaylandCompositor::WaylandCompositor::defaultOutput
 *
 * This property contains the first in the list of outputs added to the
 * WaylandCompositor, or null if no outputs have been added.
 *
 * Setting a new default output prepends it to the output list, making
 * it the new default, but the previous default is not removed from
 * the list.
 */
/*!
 * \property WaylandCompositor::defaultOutput
 *
 * This property contains the first in the list of outputs added to the
 * WaylandCompositor, or null if no outputs have been added.
 *
 * Setting a new default output prepends it to the output list, making
 * it the new default, but the previous default is not removed from
 * the list. If the new default output was already in the list of outputs,
 * it is moved to the beginning of the list.
 */
WaylandOutput *WaylandCompositor::defaultOutput() const
{
    Q_D(const WaylandCompositor);
    return d->defaultOutput();
}

void WaylandCompositor::setDefaultOutput(WaylandOutput *output)
{
    Q_D(WaylandCompositor);
    if (d->outputs.size() && d->outputs.first() == output)
        return;
    bool alreadyAdded = d->outputs.removeOne(output);
    d->outputs.prepend(output);
    emit defaultOutputChanged();
    if (!alreadyAdded)
        emit outputAdded(output);
}

/*!
 * \internal
 */
QList<WaylandOutput *> WaylandCompositor::outputs() const
{
    Q_D(const WaylandCompositor);
    return d->outputs;
}

/*!
 * \internal
 */
uint WaylandCompositor::currentTimeMsecs() const
{
    Q_D(const WaylandCompositor);
    return d->timer.elapsed();
}

/*!
 * \internal
 */
void WaylandCompositor::processWaylandEvents()
{
    Q_D(WaylandCompositor);
    int ret = wl_event_loop_dispatch(d->loop, 0);
    if (ret)
        fprintf(stderr, "wl_event_loop_dispatch error: %d\n", ret);
    wl_display_flush_clients(d->display);
}

/*!
 * \internal
 */
WaylandSeat *WaylandCompositor::createSeat()
{
    return new WaylandSeat(this);
}

/*!
 * \internal
 */
WaylandPointer *WaylandCompositor::createPointerDevice(WaylandSeat *seat)
{
    return new WaylandPointer(seat);
}

/*!
 * \internal
 */
WaylandKeyboard *WaylandCompositor::createKeyboardDevice(WaylandSeat *seat)
{
    return new WaylandKeyboard(seat);
}

/*!
 * \internal
 */
WaylandTouch *WaylandCompositor::createTouchDevice(WaylandSeat *seat)
{
    return new WaylandTouch(seat);
}

/*!
 * \qmlproperty bool QtWaylandCompositor::WaylandCompositor::retainedSelection
 *
 * This property holds whether retained selection is enabled.
 */

/*!
 * \property WaylandCompositor::retainedSelection
 *
 * This property holds whether retained selection is enabled.
 */
void WaylandCompositor::setRetainedSelectionEnabled(bool enabled)
{
    Q_D(WaylandCompositor);

    if (d->retainSelection == enabled)
        return;

    d->retainSelection = enabled;
    emit retainedSelectionChanged(enabled);
}

bool WaylandCompositor::retainedSelectionEnabled() const
{
    Q_D(const WaylandCompositor);
    return d->retainSelection;
}

/*!
 * \internal
 */
void WaylandCompositor::retainedSelectionReceived(QMimeData *)
{
}

/*!
 * \internal
 */
void WaylandCompositor::overrideSelection(const QMimeData *data)
{
    Q_D(WaylandCompositor);
#if LIRI_FEATURE_aurora_datadevice
    d->data_device_manager->overrideSelection(*data);
#endif
}

/*!
 * \qmlproperty WaylandSeat QtWaylandCompositor::WaylandCompositor::defaultSeat
 *
 * This property contains the default seat for this
 * WaylandCompositor.
 */

/*!
 * \property WaylandCompositor::defaultSeat
 *
 * This property contains the default seat for this
 * WaylandCompositor.
 */
WaylandSeat *WaylandCompositor::defaultSeat() const
{
    Q_D(const WaylandCompositor);
    if (d->seats.size())
        return d->seats.first();
    return nullptr;
}

/*!
 * Select the seat for a given input event \a inputEvent.
 * Currently, Qt only supports a single seat, but you can reimplement
 * WaylandCompositorPrivate::seatFor for a custom seat selection.
 */
WaylandSeat *WaylandCompositor::seatFor(QInputEvent *inputEvent)
{
    Q_D(WaylandCompositor);
    return d->seatFor(inputEvent);
}

/*!
 * \qmlproperty bool QtWaylandCompositor::WaylandCompositor::useHardwareIntegrationExtension
 *
 * This property holds whether the hardware integration extension should be enabled for
 * this WaylandCompositor.
 *
 * This property must be set before the compositor component is completed.
 */

/*!
 * \property WaylandCompositor::useHardwareIntegrationExtension
 *
 * This property holds whether the hardware integration extension should be enabled for
 * this WaylandCompositor.
 *
 * This property must be set before the compositor is \l{create()}{created}.
 */
bool WaylandCompositor::useHardwareIntegrationExtension() const
{
#if QT_CONFIG(opengl)
    Q_D(const WaylandCompositor);
    return d->use_hw_integration_extension;
#else
    return false;
#endif
}

void WaylandCompositor::setUseHardwareIntegrationExtension(bool use)
{
#if QT_CONFIG(opengl)
    Q_D(WaylandCompositor);
    if (use == d->use_hw_integration_extension)
        return;

    if (d->initialized)
        qWarning("Setting WaylandCompositor::useHardwareIntegrationExtension after initialization has no effect");

    d->use_hw_integration_extension = use;
    emit useHardwareIntegrationExtensionChanged();
#else
    if (use)
        qWarning() << "Hardware integration not supported without OpenGL support";
#endif
}

/*!
 * Grab the surface content from the given \a buffer.
 * The default implementation requires a OpenGL context to be bound to the current thread
 * to work. If this is not possible, reimplement this function in your compositor subclass
 * to implement custom logic.
 * The default implementation only grabs shared memory and OpenGL buffers, reimplement this in your
 * compositor subclass to handle more buffer types.
 * \note You should not call this manually, but rather use WaylandSurfaceGrabber (\a grabber).
 */
void WaylandCompositor::grabSurface(WaylandSurfaceGrabber *grabber, const WaylandBufferRef &buffer)
{
    if (buffer.isSharedMemory()) {
        emit grabber->success(buffer.image());
    } else {
#if QT_CONFIG(opengl)
        if (QOpenGLContext::currentContext()) {
            QOpenGLFramebufferObject fbo(buffer.size());
            fbo.bind();
            QOpenGLTextureBlitter blitter;
            blitter.create();


            glViewport(0, 0, buffer.size().width(), buffer.size().height());

            QOpenGLTextureBlitter::Origin surfaceOrigin =
                buffer.origin() == WaylandSurface::OriginTopLeft
                ? QOpenGLTextureBlitter::OriginTopLeft
                : QOpenGLTextureBlitter::OriginBottomLeft;

            auto texture = buffer.toOpenGLTexture();
            blitter.bind(texture->target());
            blitter.blit(texture->textureId(), QMatrix4x4(), surfaceOrigin);
            blitter.release();

            emit grabber->success(fbo.toImage());
        } else
#endif
        emit grabber->failed(WaylandSurfaceGrabber::UnknownBufferType);
    }
}

/*!
 * \qmlproperty list<enum> QtWaylandCompositor::WaylandCompositor::additionalShmFormats
 *
 * This property holds the list of additional wl_shm formats advertised as supported by the
 * compositor.
 *
 * By default, only the required ShmFormat_ARGB8888 and ShmFormat_XRGB8888 are listed and this
 * list will empty. Additional formats may require conversion internally and can thus affect
 * performance.
 *
 * This property must be set before the compositor component is completed. Subsequent changes
 * will have no effect.
 *
 * \since 6.0
 */

/*!
 * \property WaylandCompositor::additionalShmFormats
 *
 * This property holds the list of additional wl_shm formats advertised as supported by the
 * compositor.
 *
 * By default, only the required ShmFormat_ARGB8888 and ShmFormat_XRGB8888 are listed and this
 * list will empty.
 *
 * This property must be set before the compositor is \l{create()}{created}. Subsequent changes
 * will have no effect.
 *
 * \since 6.0
 */
void WaylandCompositor::setAdditionalShmFormats(const QVector<ShmFormat> &additionalShmFormats)
{
    Q_D(WaylandCompositor);
    if (d->initialized)
        qCWarning(qLcWaylandCompositorHardwareIntegration) << "Setting WaylandCompositor::additionalShmFormats after initialization has no effect";

    d->shmFormats = additionalShmFormats;
    emit additionalShmFormatsChanged();
}

QVector<WaylandCompositor::ShmFormat> WaylandCompositor::additionalShmFormats() const
{
    Q_D(const WaylandCompositor);
    return d->shmFormats;
}

void WaylandCompositor::applicationStateChanged(Qt::ApplicationState state)
{
#if LIRI_FEATURE_aurora_xkbcommon
    if (state == Qt::ApplicationInactive) {
        auto *seat = defaultSeat();
        if (seat != nullptr) {
            WaylandKeyboardPrivate *keyb = WaylandKeyboardPrivate::get(seat->keyboard());
            keyb->resetKeyboardState();
        }
    }
#else
    Q_UNUSED(state);
#endif
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandcompositor.cpp"
