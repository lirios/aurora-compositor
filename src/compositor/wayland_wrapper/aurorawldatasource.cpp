// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawldatasource_p.h"
#include "aurorawldataoffer_p.h"
#include "aurorawldatadevice_p.h"
#include "aurorawldatadevicemanager_p.h"
#include <LiriAuroraCompositor/private/aurorawaylandutils_p.h>

#include <unistd.h>
#include <LiriAuroraCompositor/private/wayland-wayland-server-protocol.h>

namespace Aurora {

namespace Compositor {

namespace Internal {

DataSource::DataSource(struct wl_client *client, uint32_t id, uint32_t time)
    : PrivateServer::wl_data_source(client, id, 1)
    , m_time(time)
{
}

DataSource::~DataSource()
{
    if (m_manager)
        m_manager->sourceDestroyed(this);
    if (m_device)
        m_device->sourceDestroyed(this);
}

uint32_t DataSource::time() const
{
    return m_time;
}

QList<QString> DataSource::mimeTypes() const
{
    return m_mimeTypes;
}

void DataSource::accept(const QString &mimeType)
{
    send_target(mimeType);
}

void DataSource::send(const QString &mimeType, int fd)
{
    send_send(mimeType, fd);
    close(fd);
}

void DataSource::cancel()
{
    send_cancelled();
}

void DataSource::setManager(DataDeviceManager *mgr)
{
    m_manager = mgr;
}

void DataSource::setDevice(DataDevice *device)
{
    m_device = device;
}

DataSource *DataSource::fromResource(struct ::wl_resource *resource)
{
    return Internal::fromResource<DataSource *>(resource);
}

void DataSource::data_source_offer(Resource *, const QString &mime_type)
{
    m_mimeTypes.append(mime_type);
}

void DataSource::data_source_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void DataSource::data_source_destroy_resource(PrivateServer::wl_data_source::Resource *resource)
{
    Q_UNUSED(resource);
    delete this;
}

}

} // namespace Compositor

} // namespace Aurora
