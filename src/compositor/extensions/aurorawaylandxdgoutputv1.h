/****************************************************************************
**
** Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef AURORA_COMPOSITOR_WAYLANDXDGOUTPUTV1_H
#define AURORA_COMPOSITOR_WAYLANDXDGOUTPUTV1_H

#include <QRect>
#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>

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
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandXdgOutputV1)
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
