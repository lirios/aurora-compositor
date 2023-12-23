// Copyright (C) 2017-2016 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
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

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/WaylandOutput>
#include <LiriAuroraCompositor/WaylandClient>
#include <LiriAuroraCompositor/WaylandSurface>
#include <LiriAuroraCompositor/WaylandXdgOutputV1>

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>

#include <QtCore/QList>
#include <QtCore/QRect>

namespace Aurora {

namespace Compositor {

struct WaylandSurfaceViewMapper
{
    WaylandSurfaceViewMapper()
    {}

    WaylandSurfaceViewMapper(WaylandSurface *s, WaylandView *v)
        : surface(s)
        , views(1, v)
    {}

    WaylandView *maybePrimaryView() const
    {
        for (int i = 0; i < views.size(); i++) {
            if (surface && surface->primaryView() == views.at(i))
                return views.at(i);
        }
        return nullptr;
    }

    WaylandSurface *surface = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<WaylandView *> views;
#else
    QVector<WaylandView *> views;
#endif
    bool has_entered = false;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandOutputPrivate : public PrivateServer::wl_output
{
    Q_DECLARE_PUBLIC(WaylandOutput)
public:
    WaylandOutputPrivate(WaylandOutput *self);

    ~WaylandOutputPrivate() override;
    static WaylandOutputPrivate *get(WaylandOutput *output) { return output->d_func(); }

    void addView(WaylandView *view, WaylandSurface *surface);
    void removeView(WaylandView *view, WaylandSurface *surface);

    void sendGeometry(const Resource *resource);
    void sendGeometryInfo();

    void sendMode(const Resource *resource, const WaylandOutputMode &mode);
    void sendModesInfo();

    void handleWindowPixelSizeChanged();

    QPointer<WaylandXdgOutputV1> xdgOutput;

protected:
    void output_bind_resource(Resource *resource) override;

private:
    void _q_handleMaybeWindowPixelSizeChanged();
    void _q_handleWindowDestroyed();

    WaylandOutput *q_ptr = nullptr;
    WaylandCompositor *compositor = nullptr;
    QWindow *window = nullptr;
    QString manufacturer;
    QString model;
    QPoint position;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<WaylandOutputMode> modes;
#else
    QVector<WaylandOutputMode> modes;
#endif
    int currentMode = -1;
    int preferredMode = -1;
    QRect availableGeometry;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<WaylandSurfaceViewMapper> surfaceViews;
#else
    QVector<WaylandSurfaceViewMapper> surfaceViews;
#endif
    QSize physicalSize;
    WaylandOutput::Subpixel subpixel = WaylandOutput::SubpixelUnknown;
    WaylandOutput::Transform transform = WaylandOutput::TransformNormal;
    int scaleFactor = 1;
    bool sizeFollowsWindow = false;
    bool initialized = false;
    QSize windowPixelSize;

    Q_DISABLE_COPY(WaylandOutputPrivate)

    friend class WaylandXdgOutputManagerV1Private;
};


} // namespace Compositor

} // namespace Aurora

