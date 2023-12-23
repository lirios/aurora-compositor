// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QRect>

#include <LiriAuroraCompositor/WaylandWlrScreencopyManagerV1>
#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-wlr-screencopy-unstable-v1.h>

#include <wayland-server-protocol.h>

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

class WaylandWlrScreencopyFrameEventFilter;

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrScreencopyManagerV1Private
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::zwlr_screencopy_manager_v1
{
    Q_DECLARE_PUBLIC(WaylandWlrScreencopyManagerV1)
public:
    explicit WaylandWlrScreencopyManagerV1Private(WaylandWlrScreencopyManagerV1 *self);

protected:
    void zwlr_screencopy_manager_v1_capture_output(Resource *resource,
                                                   uint32_t frame,
                                                   int32_t overlay_cursor,
                                                   struct ::wl_resource *output_res) override;
    void zwlr_screencopy_manager_v1_capture_output_region(Resource *resource,
                                                          uint32_t frame,
                                                          int32_t overlay_cursor,
                                                          struct ::wl_resource *output_res,
                                                          int32_t x, int32_t y,
                                                          int32_t width, int32_t height) override;
    void zwlr_screencopy_manager_v1_destroy(Resource *resource) override;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrScreencopyFrameV1Private
        : public PrivateServer::zwlr_screencopy_frame_v1
{
    Q_DECLARE_PUBLIC(WaylandWlrScreencopyFrameV1)
public:
    explicit WaylandWlrScreencopyFrameV1Private(WaylandWlrScreencopyFrameV1 *self);
    ~WaylandWlrScreencopyFrameV1Private();

    void setup();
    void copy(Resource *resource, struct ::wl_resource *buffer_res);

    WaylandWlrScreencopyFrameEventFilter *filterObject = nullptr;
    bool overlayCursor = false;
    WaylandOutput *output = nullptr;
    QRect rect;
    WaylandWlrScreencopyFrameV1::Flags flags;
    bool withDamage = false;
    QRect damageRect;
    uint32_t stride = 0;
    wl_shm_format requestedBufferFormat = WL_SHM_FORMAT_ABGR8888;
    struct ::wl_shm_buffer *buffer = nullptr;
    quint32 tv_sec_hi = 0, tv_sec_lo = 0;
    bool ready = false;

    static WaylandWlrScreencopyFrameV1Private *get(WaylandWlrScreencopyFrameV1 *frame) { return frame ? frame->d_func() : nullptr; }

protected:
    WaylandWlrScreencopyFrameV1 *q_ptr;

    void zwlr_screencopy_frame_v1_destroy_resource(Resource *resource) override;
    void zwlr_screencopy_frame_v1_copy(Resource *resource,
                                       struct ::wl_resource *buffer_res) override;
    void zwlr_screencopy_frame_v1_destroy(Resource *resource) override;
    void zwlr_screencopy_frame_v1_copy_with_damage(Resource *resource,
                                                   struct ::wl_resource *buffer_res) override;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrScreencopyFrameEventFilter
        : public QObject
{
    Q_OBJECT
public:
    explicit WaylandWlrScreencopyFrameEventFilter(QObject *parent = nullptr);

    WaylandWlrScreencopyFrameV1Private::Resource *resource = nullptr;
    struct ::wl_resource *buffer_res = nullptr;
    WaylandWlrScreencopyFrameV1Private *frame = nullptr;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

} // namespace Compositor

} // namespace Aurora

