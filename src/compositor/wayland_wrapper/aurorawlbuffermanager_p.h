// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWLBUFFERMANAGER_H
#define QWLBUFFERMANAGER_H

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

#include <QtCore/QObject>
#include <QtCore/QHash>
#include "aurorawlclientbuffer_p.h"
namespace Aurora {

namespace Compositor {

class WaylandCompositor;

namespace Internal {

class ClientBuffer;

class LIRIAURORACOMPOSITOR_EXPORT BufferManager : public QObject
{
public:
    BufferManager(WaylandCompositor *compositor);
    ClientBuffer *getBuffer(struct ::wl_resource *buffer_resource);
    void registerBuffer(struct ::wl_resource *buffer_resource, ClientBuffer *clientBuffer);
private:
    friend struct buffer_manager_destroy_listener;
    static void destroy_listener_callback(wl_listener *listener, void *data);

    QHash<struct ::wl_resource *, ClientBuffer*> m_buffers;
    WaylandCompositor *m_compositor = nullptr;
};

}
} // namespace Compositor

} // namespace Aurora

#endif // QWLBUFFERMANAGER_H
