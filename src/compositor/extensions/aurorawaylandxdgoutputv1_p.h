// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QtCore/QHash>

#include <LiriAuroraCompositor/WaylandOutput>
#include <LiriAuroraCompositor/WaylandXdgOutputV1>
#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-xdg-output-unstable-v1.h>

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

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgOutputManagerV1Private
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::zxdg_output_manager_v1
{
    Q_DECLARE_PUBLIC(WaylandXdgOutputManagerV1)
public:
    explicit WaylandXdgOutputManagerV1Private() = default;

    void registerXdgOutput(WaylandOutput *output, WaylandXdgOutputV1 *xdgOutput);
    void unregisterXdgOutput(WaylandOutput *output);

    static WaylandXdgOutputManagerV1Private *get(WaylandXdgOutputManagerV1 *manager) { return manager ? manager->d_func() : nullptr; }

protected:
    void zxdg_output_manager_v1_get_xdg_output(Resource *resource, uint32_t id,
                                               wl_resource *outputResource) override;

private:
    QHash<WaylandOutput *, WaylandXdgOutputV1 *> xdgOutputs;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgOutputV1Private
        : public QObjectPrivate
        , public PrivateServer::zxdg_output_v1
{
    Q_DECLARE_PUBLIC(WaylandXdgOutputV1)
public:
    explicit WaylandXdgOutputV1Private() = default;

    void sendLogicalPosition(const QPoint &position);
    void sendLogicalSize(const QSize &size);
    void sendDone();

    void setManager(WaylandXdgOutputManagerV1 *manager);
    void setOutput(WaylandOutput *output);

    static WaylandXdgOutputV1Private *get(WaylandXdgOutputV1 *xdgOutput) { return xdgOutput ? xdgOutput->d_func() : nullptr; }

    bool initialized = false;
    WaylandOutput *output = nullptr;
    WaylandXdgOutputManagerV1 *manager = nullptr;
    QPoint logicalPos;
    QSize logicalSize;
    QString name;
    QString description;
    bool needToSendDone = false;

protected:
    void zxdg_output_v1_bind_resource(Resource *resource) override;
    void zxdg_output_v1_destroy(Resource *resource) override;
};

} // namespace Compositor

} // namespace Aurora

