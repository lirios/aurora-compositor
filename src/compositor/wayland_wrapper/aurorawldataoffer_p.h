// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef WLDATAOFFER_H
#define WLDATAOFFER_H

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

#include <QPointer>
#include <LiriAuroraCompositor/private/aurora-server-wayland.h>

namespace Aurora {

namespace Compositor {

namespace Internal
{

class DataSource;

class DataOffer : public PrivateServer::wl_data_offer
{
public:
    DataOffer(DataSource *data_source, PrivateServer::wl_data_device::Resource *target);
    ~DataOffer() override;

protected:
    void data_offer_accept(Resource *resource, uint32_t serial, const QString &mime_type) override;
    void data_offer_receive(Resource *resource, const QString &mime_type, int32_t fd) override;
    void data_offer_destroy(Resource *resource) override;
    void data_offer_destroy_resource(Resource *resource) override;

private:
    QPointer<DataSource> m_dataSource;
};

}

} // namespace Compositor

} // namespace Aurora

#endif // WLDATAOFFER_H
