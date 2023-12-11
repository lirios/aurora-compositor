// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/private/aurorawlhardwarelayerintegration_p.h>
#include <private/qobject_p.h>

#include <QPoint>
#include <QSize>

struct wl_kms_buffer;

namespace Aurora {

namespace Compositor {

namespace QNativeInterface::Private {
struct QVsp2Screen;
}

class QScreen;
class WaylandSurface;
class WaylandQuickHardwareLayer;

class Vsp2Layer;

class Vsp2HardwareLayerIntegration : public Internal::HardwareLayerIntegration
{
    Q_OBJECT
public:
    explicit Vsp2HardwareLayerIntegration();

    void add(WaylandQuickHardwareLayer *layer) override;
    void remove(WaylandQuickHardwareLayer *layer) override;

    void sendFrameCallbacks();
    QList<QSharedPointer<Vsp2Layer>> m_layers;
private:
    void enableVspLayers();
    void disableVspLayers();
    void sortLayersByDepth();
    void recreateVspLayers();
    friend class Vsp2Layer;
};

struct Vsp2Buffer
{
    explicit Vsp2Buffer() = default;
    explicit Vsp2Buffer(wl_kms_buffer *kmsBuffer);

    int dmabufFd = -1;
    uint bytesPerLine = 0;
    uint drmPixelFormat = 0;
    QSize size;
};

class Vsp2Layer : public QObject
{
    Q_OBJECT
public:
    explicit Vsp2Layer(WaylandQuickHardwareLayer *m_hwLayer, Vsp2HardwareLayerIntegration *integration);
    void enableVspLayer();
    void disableVspLayer();
    bool isEnabled() { return m_layerIndex != -1; }
    WaylandQuickHardwareLayer *hwLayer() const { return m_hwLayer; }

public Q_SLOTS:
    void handleBufferCommitted();
    void handleSurfaceChanged();
    void updatePosition();
    void updateOpacity();

private:
    wl_kms_buffer *nextKmsBuffer();
    int m_layerIndex = -1;
    QVsp2Screen *m_screen = nullptr;
    QPoint m_position;
    WaylandQuickHardwareLayer *m_hwLayer = nullptr;
    WaylandSurface *m_surface = nullptr;
    Vsp2Buffer m_buffer;
};

} // namespace Compositor

} // namespace Aurora

