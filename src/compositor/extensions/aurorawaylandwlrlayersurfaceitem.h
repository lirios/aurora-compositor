// SPDX-FileCopyrightText: 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AURORA_COMPOSITOR_WAYLANDWLRLAYERSURFACEITEM_H
#define AURORA_COMPOSITOR_WAYLANDWLRLAYERSURFACEITEM_H

#include <LiriAuroraCompositor/WaylandQuickShellSurfaceItem>

namespace Aurora {

namespace Compositor {

class WaylandWlrLayerSurfaceV1;
class WaylandWlrLayerSurfaceItemPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrLayerSurfaceItem : public WaylandQuickShellSurfaceItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrLayerSurfaceItem)
    Q_PROPERTY(Aurora::Compositor::WaylandWlrLayerSurfaceV1 *layerSurface READ layerSurface WRITE setLayerSurface NOTIFY layerSurfaceChanged)
    QML_ELEMENT
public:
    WaylandWlrLayerSurfaceItem(QQuickItem *parent = nullptr);
    ~WaylandWlrLayerSurfaceItem();

    WaylandWlrLayerSurfaceV1 *layerSurface() const;
    void setLayerSurface(WaylandWlrLayerSurfaceV1 *layerSurface);

Q_SIGNALS:
    void layerSurfaceChanged(Aurora::Compositor::WaylandWlrLayerSurfaceV1 *layerSurface);

private:
    QScopedPointer<WaylandWlrLayerSurfaceItemPrivate> const d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_configure());
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDWLRLAYERSURFACEITEM_H
