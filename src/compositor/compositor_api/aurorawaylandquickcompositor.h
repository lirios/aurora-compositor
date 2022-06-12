/****************************************************************************
**
** Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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

#ifndef AURORA_COMPOSITOR_WAYLANDQUICKCOMPOSITOR_H
#define AURORA_COMPOSITOR_WAYLANDQUICKCOMPOSITOR_H

#include <LiriAuroraCompositor/aurorawaylandcompositor.h>
#include <QtQml/QQmlListProperty>
#include <QtQml/QQmlParserStatus>

class QQuickWindow;

namespace Aurora {

namespace Compositor {

class WaylandQuickCompositorPrivate;
class WaylandView;

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickCompositor : public WaylandCompositor, public QQmlParserStatus
{
    Q_INTERFACES(QQmlParserStatus)
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Aurora::Compositor::WaylandOutput> outputs READ outputsListProperty DESIGNABLE false CONSTANT)
    Q_INTERFACES(QQmlParserStatus)
public:
    WaylandQuickCompositor(QObject *parent = nullptr);
    ~WaylandQuickCompositor();

    void create() override;

    void grabSurface(WaylandSurfaceGrabber *grabber, const WaylandBufferRef &buffer) override;

    QQmlListProperty<WaylandOutput> outputsListProperty();

    static int countFunction(QQmlListProperty<WaylandOutput> *list);
    static WaylandOutput *atFunction(QQmlListProperty<WaylandOutput> *list, int index);

protected:
    void classBegin() override;
    void componentComplete() override;

private:
    QScopedPointer<WaylandQuickCompositorPrivate> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

#endif
