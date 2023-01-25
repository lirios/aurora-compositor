// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDXDGOUTPUTV1_H
#define AURORA_COMPOSITOR_WAYLANDXDGOUTPUTV1_H

#include <QtCore/QRect>
#include <LiriAuroraCompositor/WaylandCompositorExtension>
#if LIRI_FEATURE_aurora_compositor_quick
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>
#endif

namespace Aurora {

namespace Compositor {

class WaylandOutput;

class WaylandXdgOutputManagerV1Private;
class WaylandXdgOutputV1Private;

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgOutputManagerV1
        : public WaylandCompositorExtensionTemplate<WaylandXdgOutputManagerV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandXdgOutputManagerV1)
public:
    explicit WaylandXdgOutputManagerV1();
    WaylandXdgOutputManagerV1(WaylandCompositor *compositor);
    ~WaylandXdgOutputManagerV1();

    void initialize() override;

    static const wl_interface *interface();

private:
    QScopedPointer<WaylandXdgOutputManagerV1Private> const d_ptr;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgOutputV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandXdgOutputV1)
#if LIRI_FEATURE_aurora_compositor_quick
    AURORA_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandXdgOutputV1)
#endif
    Q_PROPERTY(Aurora::Compositor::WaylandXdgOutputManagerV1 *manager READ manager NOTIFY managerChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandOutput *output READ output NOTIFY outputChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QPoint logicalPosition READ logicalPosition WRITE setLogicalPosition NOTIFY logicalPositionChanged)
    Q_PROPERTY(QSize logicalSize READ logicalSize WRITE setLogicalSize NOTIFY logicalSizeChanged)
    Q_PROPERTY(QRect logicalGeometry READ logicalGeometry NOTIFY logicalGeometryChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    Q_MOC_INCLUDE("aurorawaylandoutput.h")
#endif
public:
    WaylandXdgOutputV1();
    WaylandXdgOutputV1(WaylandOutput *output, WaylandXdgOutputManagerV1 *manager);
    ~WaylandXdgOutputV1() override;

    WaylandXdgOutputManagerV1 *manager() const;
    WaylandOutput *output() const;

    QString name() const;
    void setName(const QString &name);

    QString description() const;
    void setDescription(const QString &name);

    QPoint logicalPosition() const;
    void setLogicalPosition(const QPoint &position);

    QSize logicalSize() const;
    void setLogicalSize(const QSize &size);

    QRect logicalGeometry() const;

Q_SIGNALS:
    void managerChanged();
    void outputChanged();
    void logicalPositionChanged();
    void logicalSizeChanged();
    void logicalGeometryChanged();
    void nameChanged();
    void descriptionChanged();

private:
    QScopedPointer<WaylandXdgOutputV1Private> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDXDGOUTPUTV1_H
