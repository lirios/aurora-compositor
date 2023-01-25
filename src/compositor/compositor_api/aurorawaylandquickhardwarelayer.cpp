// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandquickhardwarelayer_p.h"

#include <LiriAuroraCompositor/private/aurorawlhardwarelayerintegration_p.h>
#include <LiriAuroraCompositor/private/aurorawlhardwarelayerintegrationfactory_p.h>

#include <QMatrix4x4>

namespace Aurora {

namespace Compositor {

class WaylandQuickHardwareLayerPrivate
{
    Q_DECLARE_PUBLIC(WaylandQuickHardwareLayer)
public:
    WaylandQuickHardwareLayerPrivate(WaylandQuickHardwareLayer *self);

    Internal::HardwareLayerIntegration *layerIntegration();
    WaylandQuickItem *m_waylandItem = nullptr;
    int m_stackingLevel = 0;
    QMatrix4x4 m_matrixFromRenderThread;
    static Internal::HardwareLayerIntegration *s_hardwareLayerIntegration;

private:
    WaylandQuickHardwareLayer *q_ptr = nullptr;
};

Internal::HardwareLayerIntegration *WaylandQuickHardwareLayerPrivate::s_hardwareLayerIntegration = nullptr;

WaylandQuickHardwareLayerPrivate::WaylandQuickHardwareLayerPrivate(WaylandQuickHardwareLayer *self)
    : q_ptr(self)
{
}

Internal::HardwareLayerIntegration *WaylandQuickHardwareLayerPrivate::layerIntegration()
{
    if (!s_hardwareLayerIntegration) {
        QStringList keys = Internal::HardwareLayerIntegrationFactory::keys();

        QString environmentKey = QString::fromLocal8Bit(qgetenv("QT_WAYLAND_HARDWARE_LAYER_INTEGRATION").constData());
        if (!environmentKey.isEmpty()) {
            if (keys.contains(environmentKey)) {
                s_hardwareLayerIntegration = Internal::HardwareLayerIntegrationFactory::create(environmentKey, QStringList());
            } else {
                qWarning() << "Unknown hardware layer integration:" << environmentKey
                           << "Valid layer integrations are" << keys;
            }
        } else if (!keys.isEmpty()) {
            s_hardwareLayerIntegration = Internal::HardwareLayerIntegrationFactory::create(keys.first(), QStringList());
        } else {
            qWarning() << "No wayland hardware layer integrations found";
        }
    }

    return s_hardwareLayerIntegration;
}

/*!
 * \qmltype WaylandHardwareLayer
 * \inqmlmodule Aurora.Compositor
 * \preliminary
 * \brief Makes a parent WaylandQuickItem use hardware layers for rendering.
 *
 * This item needs to be a descendant of a WaylandQuickItem or a derivative,
 * (i.e. ShellSurfaceItem or similar)
 *
 * The Surface of the parent WaylandQuickItem will be drawn in a hardware specific way instead
 * of the regular way using the QtQuick scene graph. On some platforms, the WaylandQuickItem's
 * current buffer and the scene graph can be blended in a separate step. This makes it possible for
 * clients to update continuously without triggering a full redraw of the compositor scene graph for
 * each frame.
 *
 * The preferred hardware layer integration may be overridden by setting the
 * QT_WAYLAND_HARDWARE_LAYER_INTEGRATION environment variable.
 */

WaylandQuickHardwareLayer::WaylandQuickHardwareLayer(QObject *parent)
    : QObject(parent)
    , d_ptr(new WaylandQuickHardwareLayerPrivate(this))
{
}

WaylandQuickHardwareLayer::~WaylandQuickHardwareLayer()
{
    Q_D(WaylandQuickHardwareLayer);
    if (d->layerIntegration())
        d->layerIntegration()->remove(this);
}

/*!
 * \qmlproperty int AuroraCompositor::WaylandHardwareLayer::stackingLevel
 *
 * This property holds the stacking level of this hardware layer relative to other hardware layers,
 * and can be used to sort hardware layers. I.e. a layer with a higher level is rendered on top of
 * one with a lower level.
 *
 * Layers with level 0 will be drawn in an implementation defined order on top of the compositor
 * scene graph.
 *
 * Layers with a level below 0 are drawn beneath the compositor scene graph, if supported by the
 * hardware layer integration.
 */
int WaylandQuickHardwareLayer::stackingLevel() const
{
    Q_D(const WaylandQuickHardwareLayer);
    return d->m_stackingLevel;
}

void WaylandQuickHardwareLayer::setStackingLevel(int level)
{
    Q_D(WaylandQuickHardwareLayer);
    if (level == d->m_stackingLevel)
        return;

    d->m_stackingLevel = level;
    emit stackingLevelChanged();
}

WaylandQuickItem *WaylandQuickHardwareLayer::waylandItem() const
{
    Q_D(const WaylandQuickHardwareLayer);
    return d->m_waylandItem;
}

void WaylandQuickHardwareLayer::classBegin()
{
    Q_D(WaylandQuickHardwareLayer);
    for (QObject *p = parent(); p != nullptr; p = p->parent()) {
        if (auto *waylandItem = qobject_cast<WaylandQuickItem *>(p)) {
            d->m_waylandItem = waylandItem;
            break;
        }
    }
}

void WaylandQuickHardwareLayer::componentComplete()
{
    Q_D(WaylandQuickHardwareLayer);
    Q_ASSERT(d->m_waylandItem);
    if (auto integration = d->layerIntegration())
        integration->add(this);
    else
        qWarning() << "No hardware layer integration. WaylandHarwareLayer has no effect.";
}

void WaylandQuickHardwareLayer::setSceneGraphPainting(bool enable)
{
    waylandItem()->setPaintEnabled(enable);
}

// This should be called if WaylandQuickHardwareLayer used as a native instance, not a qml component.
void WaylandQuickHardwareLayer::initialize()
{
    classBegin();
    componentComplete();
}

} // namespace Compositor

} // namespace Aurora
