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

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>
#include <QObject>
#include <QtCore/QList>

namespace Aurora {

namespace Compositor {

namespace Internal {

class DataOffer;
class DataDevice;
class DataDeviceManager;

class DataSource : public QObject, public PrivateServer::wl_data_source
{
public:
    DataSource(struct wl_client *client, uint32_t id, uint32_t time);
    ~DataSource() override;
    uint32_t time() const;
    QList<QString> mimeTypes() const;

    void accept(const QString &mimeType);
    void send(const QString &mimeType,int fd);
    void cancel();

    void setManager(DataDeviceManager *mgr);
    void setDevice(DataDevice *device);

    static DataSource *fromResource(struct ::wl_resource *resource);

protected:
    void data_source_offer(Resource *resource, const QString &mime_type) override;
    void data_source_destroy(Resource *resource) override;
    void data_source_destroy_resource(Resource *resource) override;

private:
    uint32_t m_time;
    QList<QString> m_mimeTypes;

    DataDevice *m_device = nullptr;
    DataDeviceManager *m_manager = nullptr;
};

}

} // namespace Compositor

} // namespace Aurora

