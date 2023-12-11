// SPDX-FileCopyrightText: 2020-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aurorawaylandcompositor.h"
#include "aurorawaylandoutput.h"
#include "aurorawaylandwlrexportdmabufv1_p.h"

#include <sys/types.h>
#include <unistd.h>

#include <wayland-server-protocol.h>

namespace Aurora {

namespace Compositor {

/*
 * WaylandWlrExportDmabufManagerV1
 */

WaylandWlrExportDmabufManagerV1::WaylandWlrExportDmabufManagerV1()
    : WaylandCompositorExtensionTemplate<WaylandWlrExportDmabufManagerV1>()
    , d_ptr(new WaylandWlrExportDmabufManagerV1Private(this))
{
}

WaylandWlrExportDmabufManagerV1::WaylandWlrExportDmabufManagerV1(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate<WaylandWlrExportDmabufManagerV1>(compositor)
    , d_ptr(new WaylandWlrExportDmabufManagerV1Private(this))
{
}

WaylandWlrExportDmabufManagerV1::~WaylandWlrExportDmabufManagerV1()
{
}

void WaylandWlrExportDmabufManagerV1::initialize()
{
    Q_D(WaylandWlrExportDmabufManagerV1);

    WaylandCompositorExtensionTemplate::initialize();
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qCWarning(gLcAuroraCompositorWlrExportDmabufV1) << "Failed to find WaylandCompositor when initializing WaylandExportDmabufManagerV1";
        return;
    }
    d->init(compositor->display(), WaylandWlrExportDmabufManagerV1Private::interfaceVersion());
}

const wl_interface *WaylandWlrExportDmabufManagerV1::interface()
{
    return WaylandWlrExportDmabufManagerV1Private::interface();
}

QByteArray WaylandWlrExportDmabufManagerV1::interfaceName()
{
    return WaylandWlrExportDmabufManagerV1Private::interfaceName();
}

/*
 * WaylandWlrExportDmabufManagerV1Private
 */

WaylandWlrExportDmabufManagerV1Private::WaylandWlrExportDmabufManagerV1Private(WaylandWlrExportDmabufManagerV1 *self)
    : WaylandCompositorExtensionPrivate(self)
{
}

void WaylandWlrExportDmabufManagerV1Private::zwlr_export_dmabuf_manager_v1_capture_output(
        Resource *resource, uint32_t id, int32_t overlay_cursor, wl_resource *outputRes)
{
    Q_Q(WaylandWlrExportDmabufManagerV1);

    // Output
    auto *output = WaylandOutput::fromResource(outputRes);
    if (!output) {
        qCWarning(gLcAuroraCompositorWlrExportDmabufV1, "Resource wl_output@%d doesn't exist",
                  wl_resource_get_id(outputRes));
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "resource wl_output@%d doesn't exist",
                               wl_resource_get_id(outputRes));
        return;
    }

    auto *frame = new WaylandWlrExportDmabufFrameV1(q, overlay_cursor == 1, output, q);
    WaylandWlrExportDmabufFrameV1Private::get(frame)->init(resource->client(), id, resource->version());
    Q_EMIT q->outputCaptureRequested(frame);
}

/*
 * WaylandWlrExportDmabufFrameV1
 */

WaylandWlrExportDmabufFrameV1::WaylandWlrExportDmabufFrameV1(WaylandWlrExportDmabufManagerV1 *manager,
                                                             bool overlayCursor, WaylandOutput *output,
                                                             QObject *parent)
    : QObject(parent)
    , d_ptr(new WaylandWlrExportDmabufFrameV1Private(this))
{
    Q_D(WaylandWlrExportDmabufFrameV1);
    d->manager = manager;
    d->overlayCursor = overlayCursor;
    d->output = output;
}

WaylandWlrExportDmabufFrameV1::~WaylandWlrExportDmabufFrameV1()
{
}

bool WaylandWlrExportDmabufFrameV1::overlayCursor() const
{
    Q_D(const WaylandWlrExportDmabufFrameV1);
    return d->overlayCursor;
}

WaylandOutput *WaylandWlrExportDmabufFrameV1::output() const
{
    Q_D(const WaylandWlrExportDmabufFrameV1);
    return d->output;
}

void WaylandWlrExportDmabufFrameV1::frame(const QSize &size, const QPoint &offset,
                                          BufferFlags bufferFlags, FrameFlags flags,
                                          quint32 drmFormat, quint64 modifier,
                                          quint32 numObjects)
{
    Q_D(WaylandWlrExportDmabufFrameV1);

    d->send_frame(size.width(), size.height(), offset.x(), offset.y(),
                  static_cast<uint32_t>(bufferFlags),
                  static_cast<uint32_t>(flags),
                  drmFormat, modifier >> 32, modifier & 0xFFFFFFFF, numObjects);
}

void WaylandWlrExportDmabufFrameV1::object(quint32 index, quint32 fd, quint32 size,
                                           quint32 offset, quint32 stride,
                                           quint32 planeIndex)
{
    Q_D(WaylandWlrExportDmabufFrameV1);

    lseek(fd, 0, SEEK_END);
    d->send_object(index, fd, size, offset, stride, planeIndex);
}

void WaylandWlrExportDmabufFrameV1::ready(quint64 tv_sec, quint32 tv_nsec)
{
    Q_D(WaylandWlrExportDmabufFrameV1);

    uint32_t tv_sec_hi = (sizeof(tv_sec) > 4) ? tv_sec >> 32 : 0;
    uint32_t tv_sec_lo = tv_sec & 0xFFFFFFFF;
    d->send_ready(tv_sec_hi, tv_sec_lo, tv_nsec);
}

void WaylandWlrExportDmabufFrameV1::cancel(WaylandWlrExportDmabufFrameV1::CancelReason reason)
{
    Q_D(WaylandWlrExportDmabufFrameV1);

    d->send_cancel(static_cast<uint32_t>(reason));
}

/*
 * WaylandWlrExportDmabufFrameV1Private
 */

WaylandWlrExportDmabufFrameV1Private::WaylandWlrExportDmabufFrameV1Private(WaylandWlrExportDmabufFrameV1 *self)
    : PrivateServer::zwlr_export_dmabuf_frame_v1()
    , q_ptr(self)
{
}

void WaylandWlrExportDmabufFrameV1Private::zwlr_export_dmabuf_frame_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource)
    delete this;
}

void WaylandWlrExportDmabufFrameV1Private::zwlr_export_dmabuf_frame_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

} // namespace Compositor

} // namespace Aurora
