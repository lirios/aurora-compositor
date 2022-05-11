// SPDX-FileCopyrightText: 2020-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AURORA_COMPOSITOR_WAYLANDWLREXPORTDMABUFV1_P_H
#define AURORA_COMPOSITOR_WAYLANDWLREXPORTDMABUFV1_P_H

#include <QLoggingCategory>

#include <LiriAuroraCompositor/WaylandWlrExportDmabufManagerV1>
#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-wlr-export-dmabuf-unstable-v1.h>

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

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrExportDmabufManagerV1Private
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::zwlr_export_dmabuf_manager_v1
{
    Q_DECLARE_PUBLIC(WaylandWlrExportDmabufManagerV1)
public:
    explicit WaylandWlrExportDmabufManagerV1Private(WaylandWlrExportDmabufManagerV1 *self);

protected:
    WaylandWlrExportDmabufManagerV1 *q_ptr;

    void zwlr_export_dmabuf_manager_v1_capture_output(Resource *resource,
                                                      uint32_t id,
                                                      int32_t overlay_cursor,
                                                      wl_resource *outputRes) override;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrExportDmabufFrameV1Private
        : public PrivateServer::zwlr_export_dmabuf_frame_v1
{
    Q_DECLARE_PUBLIC(WaylandWlrExportDmabufFrameV1)
public:
    explicit WaylandWlrExportDmabufFrameV1Private(WaylandWlrExportDmabufFrameV1 *self);

    static WaylandWlrExportDmabufFrameV1Private *get(WaylandWlrExportDmabufFrameV1 *self) { return self->d_func(); }

    WaylandWlrExportDmabufManagerV1 *manager = nullptr;
    bool overlayCursor = false;
    WaylandOutput *output = nullptr;

protected:
    WaylandWlrExportDmabufFrameV1 *q_ptr = nullptr;

    void zwlr_export_dmabuf_frame_v1_destroy_resource(Resource *resource) override;
    void zwlr_export_dmabuf_frame_v1_destroy(Resource *resource) override;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDWLREXPORTDMABUFV1_P_H
