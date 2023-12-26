// SPDX-FileCopyrightText: 2021 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

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
    QML_NAMED_ELEMENT(WaylandSurfaceLayout)
    QML_UNCREATABLE("Cannot create instance of WaylandSurfaceLayout")
public:
    WaylandSurfaceLayout(QQuickItem *parent = nullptr);
    ~WaylandSurfaceLayout();

    WaylandOutput *output() const;
    void setOutput(WaylandOutput *output);

Q_SIGNALS:
    void outputChanged(Aurora::Compositor::WaylandOutput *output);

protected:
    void itemChange(ItemChange change,
                    const ItemChangeData &data) override;
    void geometryChange(const QRectF &newGeometry,
                        const QRectF &oldGeometry) override;
    void updatePolish() override;
    void componentComplete() override;

    virtual bool sortItems(QQuickItem *left, QQuickItem *right) = 0;

private:
    QScopedPointer<WaylandSurfaceLayoutPrivate> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

