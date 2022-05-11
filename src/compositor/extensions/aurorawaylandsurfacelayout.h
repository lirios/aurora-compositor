// SPDX-FileCopyrightText: 2021 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AURORA_COMPOSITOR_WAYLANDSURFACELAYOUT_H
#define AURORA_COMPOSITOR_WAYLANDSURFACELAYOUT_H

#include <QQuickItem>

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>

namespace Aurora {

namespace Compositor {

class WaylandOutput;
class WaylandWlrLayerSurfaceV1;
class WaylandSurfaceLayoutPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandSurfaceLayout : public QQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandSurfaceLayout)
    Q_PROPERTY(Aurora::Compositor::WaylandOutput *output READ output WRITE setOutput NOTIFY outputChanged)
    QML_ELEMENT
public:
    enum Layer {
        NoLayer = 0,
        BackgroundLayer,
        BottomLayer,
        WindowsLayer,
        TopLayer,
        OverlayLayer
    };
    Q_ENUM(Layer);

    WaylandSurfaceLayout(QQuickItem *parent = nullptr);
    ~WaylandSurfaceLayout();

    WaylandOutput *output() const;
    void setOutput(WaylandOutput *output);

Q_SIGNALS:
    void outputChanged(Aurora::Compositor::WaylandOutput *output);

protected:
    void itemChange(ItemChange change,
                    const ItemChangeData &data) override;
    void geometryChanged(const QRectF &newGeometry,
                         const QRectF &oldGeometry) override;
    void updatePolish() override;
    void componentComplete() override;

    virtual WaylandSurfaceLayout::Layer getLayer(QQuickItem *item) const;
    virtual bool sortItems(QQuickItem *left, QQuickItem *right);

private:
    QScopedPointer<WaylandSurfaceLayoutPrivate> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDSURFACELAYOUT_H
