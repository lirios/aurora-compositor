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

#ifndef AURORA_COMPOSITOR_WAYLANDCOMPOSITOR_P_H
#define AURORA_COMPOSITOR_WAYLANDCOMPOSITOR_P_H

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

#include <LiriAuroraCompositor/private/qtwaylandcompositorglobal_p.h>
#include <LiriAuroraCompositor/WaylandCompositor>
#include <QtCore/private/qobject_p.h>
#include <QtCore/QSet>
#include <QtCore/QElapsedTimer>

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>

#include <vector>

#if QT_CONFIG(xkbcommon)
#include <QtGui/private/qxkbcommon_p.h>
#endif

class QWindowSystemEventHandler;

namespace Aurora {

namespace Compositor {

namespace QtWayland {
    class HardwareIntegration;
    class ClientBufferIntegration;
    class ServerBufferIntegration;
    class DataDeviceManager;
    class BufferManager;
}

class WaylandSurface;

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandCompositorPrivate : public QObjectPrivate, public PrivateServer::wl_compositor, public PrivateServer::wl_subcompositor
{
public:
    static WaylandCompositorPrivate *get(WaylandCompositor *compositor) { return compositor->d_func(); }

    WaylandCompositorPrivate(WaylandCompositor *compositor);
    ~WaylandCompositorPrivate() override;

#if QT_CONFIG(xkbcommon)
    struct xkb_context *xkbContext() const { return mXkbContext.get(); }
#endif

    void preInit();
    void init();

    void destroySurface(WaylandSurface *surface);
    void unregisterSurface(WaylandSurface *surface);

    WaylandOutput *defaultOutput() const { return outputs.size() ? outputs.first() : nullptr; }

    inline const QList<QtWayland::ClientBufferIntegration *> clientBufferIntegrations() const;
    inline QtWayland::ServerBufferIntegration *serverBufferIntegration() const;

#if QT_CONFIG(wayland_datadevice)
    QtWayland::DataDeviceManager *dataDeviceManager() const { return data_device_manager; }
#endif
    QtWayland::BufferManager *bufferManager() const { return buffer_manager; }
    void feedRetainedSelectionData(QMimeData *data);

    WaylandPointer *callCreatePointerDevice(WaylandSeat *seat)
    { return q_func()->createPointerDevice(seat); }
    WaylandKeyboard *callCreateKeyboardDevice(WaylandSeat *seat)
    { return q_func()->createKeyboardDevice(seat); }
    WaylandTouch *callCreateTouchDevice(WaylandSeat *seat)
    { return q_func()->createTouchDevice(seat); }

    inline void addClient(WaylandClient *client);
    inline void removeClient(WaylandClient *client);

    void addPolishObject(QObject *object);

    inline void addOutput(WaylandOutput *output);
    inline void removeOutput(WaylandOutput *output);

    void connectToExternalSockets();

    virtual WaylandSeat *seatFor(QInputEvent *inputEvent);

protected:
    void compositor_create_surface(wl_compositor::Resource *resource, uint32_t id) override;
    void compositor_create_region(wl_compositor::Resource *resource, uint32_t id) override;

    void subcompositor_get_subsurface(wl_subcompositor::Resource *resource, uint32_t id, struct ::wl_resource *surface, struct ::wl_resource *parent) override;

    virtual WaylandSurface *createDefaultSurface();
protected:
    void initializeHardwareIntegration();
    void initializeExtensions();
    void initializeSeats();

    void loadClientBufferIntegration();
    void loadServerBufferIntegration();

    QByteArray socket_name;
    QList<int> externally_added_socket_fds;
    struct wl_display *display = nullptr;
    bool ownsDisplay = false;
    QVector<WaylandCompositor::ShmFormat> shmFormats;

    QList<WaylandSeat *> seats;
    QList<WaylandOutput *> outputs;

    QList<WaylandSurface *> all_surfaces;

#if QT_CONFIG(wayland_datadevice)
    QtWayland::DataDeviceManager *data_device_manager = nullptr;
#endif
    QtWayland::BufferManager *buffer_manager = nullptr;

    QElapsedTimer timer;

    wl_event_loop *loop = nullptr;

    QList<WaylandClient *> clients;

#if QT_CONFIG(opengl)
    bool use_hw_integration_extension = true;
    QScopedPointer<QtWayland::HardwareIntegration> hw_integration;
    QScopedPointer<QtWayland::ServerBufferIntegration> server_buffer_integration;
#endif
    QList<QtWayland::ClientBufferIntegration*> client_buffer_integrations;

    QScopedPointer<QWindowSystemEventHandler> eventHandler;

    bool retainSelection = false;
    bool preInitialized = false;
    bool initialized = false;
    std::vector<QPointer<QObject> > polish_objects;

#if QT_CONFIG(xkbcommon)
    QXkbCommon::ScopedXKBContext mXkbContext;
#endif

    Q_DECLARE_PUBLIC(WaylandCompositor)
    Q_DISABLE_COPY(WaylandCompositorPrivate)
};

const QList<QtWayland::ClientBufferIntegration *> WaylandCompositorPrivate::clientBufferIntegrations() const
{
    return client_buffer_integrations;
}

QtWayland::ServerBufferIntegration * WaylandCompositorPrivate::serverBufferIntegration() const
{
#if QT_CONFIG(opengl)
    return server_buffer_integration.data();
#else
    return 0;
#endif
}

void WaylandCompositorPrivate::addClient(WaylandClient *client)
{
    Q_ASSERT(!clients.contains(client));
    clients.append(client);
}

void WaylandCompositorPrivate::removeClient(WaylandClient *client)
{
    Q_ASSERT(clients.contains(client));
    clients.removeOne(client);
}

void WaylandCompositorPrivate::addOutput(WaylandOutput *output)
{
    Q_ASSERT(output);
    Q_Q(WaylandCompositor);
    if (outputs.contains(output))
        return;
    outputs.append(output);
    emit q->outputAdded(output);
}

void WaylandCompositorPrivate::removeOutput(WaylandOutput *output)
{
    Q_ASSERT(output);
    Q_ASSERT(outputs.count(output) == 1);
    Q_Q(WaylandCompositor);
    if (outputs.removeOne(output))
        emit q->outputRemoved(output);
}

} // namespace Compositor

} // namespace Aurora

#endif //QWAYLANDCOMPOSITOR_P_H
