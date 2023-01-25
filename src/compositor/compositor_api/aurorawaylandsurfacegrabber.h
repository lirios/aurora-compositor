// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDSURFACEGRABBER_H
#define AURORA_COMPOSITOR_WAYLANDSURFACEGRABBER_H

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <QtCore/QObject>

namespace Aurora {

namespace Compositor {

class WaylandSurface;
class WaylandSurfaceGrabberPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandSurfaceGrabber : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandSurfaceGrabber)
public:
    enum Error {
        InvalidSurface,
        NoBufferAttached,
        UnknownBufferType,
        RendererNotReady,
    };
    Q_ENUM(Error)

    explicit WaylandSurfaceGrabber(WaylandSurface *surface, QObject *parent = nullptr);
    ~WaylandSurfaceGrabber();

    WaylandSurface *surface() const;
    void grab();

Q_SIGNALS:
    void success(const QImage &image);
    void failed(Aurora::Compositor::WaylandSurfaceGrabber::Error error);

private:
    QScopedPointer<WaylandSurfaceGrabberPrivate> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDSURFACEGRABBER_H
