// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AURORA_COMPOSITOR_WAYLANDWLRLAYERSHELLV1_H
#define AURORA_COMPOSITOR_WAYLANDWLRLAYERSHELLV1_H

#include <QMargins>
#include <QQmlEngine>

#include <LiriAuroraCompositor/WaylandCompositorExtension>

namespace Aurora {

namespace Compositor {

class WaylandCompositor;
class WaylandOutput;
class WaylandSurface;
class WaylandSurfaceRole;
class WaylandWlrLayerShellV1Private;
class WaylandWlrLayerSurfaceV1;
class WaylandWlrLayerSurfaceV1Private;

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrLayerShellV1
        : public WaylandCompositorExtensionTemplate<WaylandWlrLayerShellV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrLayerShellV1)
public:
    enum Layer {
        BackgroundLayer = 0,
        BottomLayer = 1,
        TopLayer = 2,
        OverlayLayer = 3
    };
    Q_ENUM(Layer)

    WaylandWlrLayerShellV1();
    explicit WaylandWlrLayerShellV1(WaylandCompositor *compositor);
    ~WaylandWlrLayerShellV1();

    void initialize() override;

    Q_INVOKABLE void closeAllSurfaces();

    static const wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void layerSurfaceCreated(Aurora::Compositor::WaylandWlrLayerSurfaceV1 *layerSurface);

private:
    WaylandWlrLayerShellV1Private *const d_ptr;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrLayerSurfaceV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrLayerSurfaceV1)
    Q_PROPERTY(Aurora::Compositor::WaylandSurface *surface READ surface CONSTANT)
    Q_PROPERTY(Aurora::Compositor::WaylandOutput *output READ output CONSTANT)
    Q_PROPERTY(Aurora::Compositor::WaylandWlrLayerShellV1::Layer layer READ layer NOTIFY layerChanged)
    Q_PROPERTY(QString nameSpace READ nameSpace CONSTANT)
    Q_PROPERTY(quint32 width READ width NOTIFY sizeChanged)
    Q_PROPERTY(quint32 height READ height NOTIFY sizeChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandWlrLayerSurfaceV1::Anchors anchors READ anchors NOTIFY anchorsChanged)
    Q_PROPERTY(int exclusiveZone READ exclusiveZone NOTIFY exclusiveZoneChanged)
    Q_PROPERTY(qint32 leftMargin READ leftMargin NOTIFY leftMarginChanged)
    Q_PROPERTY(qint32 topMargin READ topMargin NOTIFY topMarginChanged)
    Q_PROPERTY(qint32 rightMargin READ rightMargin NOTIFY rightMarginChanged)
    Q_PROPERTY(qint32 bottomMargin READ bottomMargin NOTIFY bottomMarginChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandWlrLayerSurfaceV1::KeyboardInteractivity keyboardInteractivity READ keyboardInteractivity NOTIFY keyboardInteractivityChanged)
    Q_PROPERTY(bool mapped READ isMapped NOTIFY mappedChanged)
    Q_PROPERTY(bool configured READ isConfigured NOTIFY configuredChanged)
public:
    enum Anchor {
        TopAnchor = 1,
        BottomAnchor = 2,
        LeftAnchor = 4,
        RightAnchor = 8
    };
    Q_ENUM(Anchor)
    Q_DECLARE_FLAGS(Anchors, Anchor)
    Q_FLAG(Anchors)

    enum KeyboardInteractivity {
        NoKeyboardInteractivity = 0,
        ExclusiveKeyboardInteractivity = 1,
        OnDemandKeyboardInteractivity = 2
    };
    Q_ENUM(KeyboardInteractivity)

    ~WaylandWlrLayerSurfaceV1();

    WaylandSurface *surface() const;
    WaylandOutput *output() const;
    WaylandWlrLayerShellV1::Layer layer() const;
    QString nameSpace() const;

    QSize size() const;
    quint32 width() const;
    quint32 height() const;
    Anchors anchors() const;
    int exclusiveZone() const;
    QMargins margins() const;
    qint32 leftMargin() const;
    qint32 topMargin() const;
    qint32 rightMargin() const;
    qint32 bottomMargin() const;
    KeyboardInteractivity keyboardInteractivity() const;
    bool isMapped() const;
    bool isConfigured() const;

    Q_INVOKABLE quint32 sendConfigure(const QSize &size);
    Q_INVOKABLE quint32 sendConfigure(int width, int height);
    Q_INVOKABLE void close();

    static WaylandSurfaceRole *role();

Q_SIGNALS:
    void layerChanged();
    void sizeChanged();
    void anchorsChanged();
    void exclusiveZoneChanged();
    void leftMarginChanged();
    void topMarginChanged();
    void rightMarginChanged();
    void bottomMarginChanged();
    void keyboardInteractivityChanged();
    void changed();
    void mappedChanged();
    void configuredChanged();

private:
    QScopedPointer<WaylandWlrLayerSurfaceV1Private> const d_ptr;

    explicit WaylandWlrLayerSurfaceV1(WaylandSurface *surface,
                                      WaylandOutput *output,
                                      WaylandWlrLayerShellV1::Layer layer,
                                      const QString &nameSpace,
                                      QObject *parent = nullptr);

    friend class WaylandWlrLayerShellV1Private;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WaylandWlrLayerSurfaceV1::Anchors)

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDWLRLAYERSHELLV1_H
