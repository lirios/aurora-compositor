// Copyright (C) 2017-2016 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/auroraqmlinclude.h>
#include <LiriAuroraCompositor/aurorawaylandcompositorextension.h>
#include <LiriAuroraCompositor/WaylandOutputMode>
#include <QtCore/QObject>
#include <QtCore/QRect>
#include <QtCore/QSize>

struct wl_resource;

class QWindow;

namespace Aurora {

namespace Compositor {

class WaylandOutputPrivate;
class WaylandCompositor;
class WaylandSurface;
class WaylandView;
class WaylandClient;

class LIRIAURORACOMPOSITOR_EXPORT WaylandOutput : public WaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandOutput)
    Q_PROPERTY(Aurora::Compositor::WaylandCompositor *compositor READ compositor WRITE setCompositor NOTIFY compositorChanged)
    Q_PROPERTY(QWindow *window READ window WRITE setWindow NOTIFY windowChanged)
    Q_PROPERTY(QString manufacturer READ manufacturer WRITE setManufacturer NOTIFY manufacturerChanged)
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QPoint position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QRect geometry READ geometry NOTIFY geometryChanged)
    Q_PROPERTY(QRect availableGeometry READ availableGeometry WRITE setAvailableGeometry NOTIFY availableGeometryChanged)
    Q_PROPERTY(QSize physicalSize READ physicalSize WRITE setPhysicalSize NOTIFY physicalSizeChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandOutput::Subpixel subpixel READ subpixel WRITE setSubpixel NOTIFY subpixelChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandOutput::Transform transform READ transform WRITE setTransform NOTIFY transformChanged)
    Q_PROPERTY(int scaleFactor READ scaleFactor WRITE setScaleFactor NOTIFY scaleFactorChanged)
    Q_PROPERTY(bool sizeFollowsWindow READ sizeFollowsWindow WRITE setSizeFollowsWindow NOTIFY sizeFollowsWindowChanged)

    QML_NAMED_ELEMENT(WaylandOutputBase)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("Cannot create instance of WaylandOutputBase, use WaylandOutput instead")
public:
    enum Subpixel {
      SubpixelUnknown = 0,
      SubpixelNone,
      SubpixelHorizontalRgb,
      SubpixelHorizontalBgr,
      SubpixelVerticalRgb,
      SubpixelVerticalBgr
    };
    Q_ENUM(Subpixel)

    enum Transform {
        TransformNormal = 0,
        Transform90,
        Transform180,
        Transform270,
        TransformFlipped,
        TransformFlipped90,
        TransformFlipped180,
        TransformFlipped270
    };
    Q_ENUM(Transform)

    WaylandOutput();
    WaylandOutput(WaylandCompositor *compositor, QWindow *window);
    ~WaylandOutput() override;

    static WaylandOutput *fromResource(wl_resource *resource);
    struct ::wl_resource *resourceForClient(WaylandClient *client) const;

    WaylandCompositor *compositor() const;
    void setCompositor(WaylandCompositor *compositor);

    QWindow *window() const;
    void setWindow(QWindow *window);

    QString manufacturer() const;
    void setManufacturer(const QString &manufacturer);

    QString model() const;
    void setModel(const QString &model);

    QPoint position() const;
    void setPosition(const QPoint &pt);

    QList<WaylandOutputMode> modes() const;

    void addMode(const WaylandOutputMode &mode, bool preferred = false);

    WaylandOutputMode currentMode() const;
    void setCurrentMode(const WaylandOutputMode &mode);

    QRect geometry() const;

    QRect availableGeometry() const;
    void setAvailableGeometry(const QRect &availableGeometry);

    QSize physicalSize() const;
    void setPhysicalSize(const QSize &size);

    Subpixel subpixel() const;
    void setSubpixel(const Subpixel &subpixel);

    Transform transform() const;
    void setTransform(const Transform &transform);

    int scaleFactor() const;
    void setScaleFactor(int scale);

    bool sizeFollowsWindow() const;
    void setSizeFollowsWindow(bool follow);

    bool physicalSizeFollowsSize() const;
    void setPhysicalSizeFollowsSize(bool follow);

    void frameStarted();
    void sendFrameCallbacks();

    void surfaceEnter(WaylandSurface *surface);
    void surfaceLeave(WaylandSurface *surface);

    virtual void update();

Q_SIGNALS:
    void compositorChanged();
    void windowChanged();
    void positionChanged();
    void geometryChanged();
    void modeAdded();
    void currentModeChanged();
    void availableGeometryChanged();
    void physicalSizeChanged();
    void scaleFactorChanged();
    void subpixelChanged();
    void transformChanged();
    void sizeFollowsWindowChanged();
    void physicalSizeFollowsSizeChanged();
    void manufacturerChanged();
    void modelChanged();
    void windowDestroyed();

protected:
    bool event(QEvent *event) override;

    virtual void initialize();

private:
    Q_PRIVATE_SLOT(d_func(), void _q_handleMaybeWindowPixelSizeChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_handleWindowDestroyed())
};

} // namespace Compositor

} // namespace Aurora

