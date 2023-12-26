// SPDX-FileCopyrightText: 2020-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <LiriAuroraCompositor/WaylandCompositorExtension>

namespace Aurora {

namespace Compositor {

class WaylandCompositor;
class WaylandOutput;
class WaylandWlrExportDmabufManagerV1Private;
class WaylandWlrExportDmabufFrameV1;
class WaylandWlrExportDmabufFrameV1Private;

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrExportDmabufManagerV1
        : public WaylandCompositorExtensionTemplate<WaylandWlrExportDmabufManagerV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrExportDmabufManagerV1)
public:
    WaylandWlrExportDmabufManagerV1();
    WaylandWlrExportDmabufManagerV1(WaylandCompositor *compositor);

    void initialize() override;

    static const wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void outputCaptureRequested(Aurora::Compositor::WaylandWlrExportDmabufFrameV1 *frame);
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrExportDmabufFrameV1
        : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrExportDmabufFrameV1)
    Q_PROPERTY(bool overlayCursor READ overlayCursor CONSTANT)
    Q_PROPERTY(Aurora::Compositor::WaylandOutput *output READ output CONSTANT)
public:
    enum BufferFlag {
        YInvert = 1,
        Interlaced = 2,
        BottomFirst = 4
    };
    Q_ENUM(BufferFlag)
    Q_DECLARE_FLAGS(BufferFlags, BufferFlag)
    Q_FLAG(BufferFlags)

    enum FrameFlag {
        Transient = 1
    };
    Q_ENUM(FrameFlag)
    Q_DECLARE_FLAGS(FrameFlags, FrameFlag)
    Q_FLAG(FrameFlags)

    enum CancelReason {
        Temporary = 0,
        Permanent,
        Resizing
    };
    Q_ENUM(CancelReason)

    ~WaylandWlrExportDmabufFrameV1();

    bool overlayCursor() const;
    WaylandOutput *output() const;

    Q_INVOKABLE void frame(const QSize &size, const QPoint &offset,
                           WaylandWlrExportDmabufFrameV1::BufferFlags bufferFlags,
                           WaylandWlrExportDmabufFrameV1::FrameFlags flags,
                           quint32 drmFormat, quint64 modifier,
                           quint32 numObjects);
    Q_INVOKABLE void object(quint32 index, quint32 fd, quint32 size,
                            quint32 offset, quint32 stride,
                            quint32 planeIndex);
    Q_INVOKABLE void ready(quint64 tv_sec, quint32 tv_nsec);
    Q_INVOKABLE void cancel(WaylandWlrExportDmabufFrameV1::CancelReason reason);

private:
    explicit WaylandWlrExportDmabufFrameV1(WaylandWlrExportDmabufManagerV1 *manager,
                                           bool overlayCursor,
                                           WaylandOutput *output,
                                           QObject *parent = nullptr);

    friend class WaylandWlrExportDmabufManagerV1Private;
};

} // namespace Compositor

} // namespace Aurora

Q_DECLARE_OPERATORS_FOR_FLAGS(Aurora::Compositor::WaylandWlrExportDmabufFrameV1::BufferFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(Aurora::Compositor::WaylandWlrExportDmabufFrameV1::FrameFlags)

