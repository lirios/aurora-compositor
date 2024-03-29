// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

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

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtGui/QClipboard>
#include <QtCore/QMimeData>

#include <LiriAuroraCompositor/WaylandCompositor>

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>

class QSocketNotifier;

namespace Aurora {

namespace Compositor {

namespace Internal {

class DataDevice;
class DataSource;

class DataDeviceManager : public QObject, public PrivateServer::wl_data_device_manager
{
    Q_OBJECT

public:
    DataDeviceManager(WaylandCompositor *compositor);

    void setCurrentSelectionSource(DataSource *source);
    DataSource *currentSelectionSource();

    struct wl_display *display() const;

    void sourceDestroyed(DataSource *source);

    void overrideSelection(const QMimeData &mimeData);
    bool offerFromCompositorToClient(wl_resource *clientDataDeviceResource);
    void offerRetainedSelection(wl_resource *clientDataDeviceResource);

protected:
    void data_device_manager_create_data_source(Resource *resource, uint32_t id) override;
    void data_device_manager_get_data_device(Resource *resource, uint32_t id, struct ::wl_resource *seat) override;

private Q_SLOTS:
    void readFromClient(int fd);

private:
    void retain();
    void finishReadFromClient(bool exhausted = false);

    WaylandCompositor *m_compositor = nullptr;
    QList<DataDevice *> m_data_device_list;

    DataSource *m_current_selection_source = nullptr;

    QMimeData m_retainedData;
    QSocketNotifier *m_retainedReadNotifier = nullptr;
    QList<QSocketNotifier *> m_obsoleteRetainedReadNotifiers;
    int m_retainedReadIndex = 0;
    QByteArray m_retainedReadBuf;

    bool m_compositorOwnsSelection = false;


    static void comp_accept(struct wl_client *client,
                            struct wl_resource *resource,
                            uint32_t time,
                            const char *type);
    static void comp_receive(struct wl_client *client,
                             struct wl_resource *resource,
                             const char *mime_type,
                             int32_t fd);
    static void comp_destroy(struct wl_client *client,
                             struct wl_resource *resource);

    static const struct wl_data_offer_interface compositor_offer_interface;
};

}

} // namespace Compositor

} // namespace Aurora

