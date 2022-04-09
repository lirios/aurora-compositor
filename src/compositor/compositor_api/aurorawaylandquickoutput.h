/****************************************************************************
**
** Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef AURORA_COMPOSITOR_WAYLANDQUICKOUTPUT_H
#define AURORA_COMPOSITOR_WAYLANDQUICKOUTPUT_H

#include <QtQuick/QQuickWindow>
#include <LiriAuroraCompositor/aurorawaylandoutput.h>
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>

QT_REQUIRE_CONFIG(wayland_compositor_quick);

namespace Aurora {

namespace Compositor {

class WaylandQuickCompositor;
class QQuickWindow;

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandQuickOutput : public WaylandOutput, public QQmlParserStatus
{
    Q_INTERFACES(QQmlParserStatus)
    Q_OBJECT
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandQuickOutput)
    Q_PROPERTY(bool automaticFrameCallback READ automaticFrameCallback WRITE setAutomaticFrameCallback NOTIFY automaticFrameCallbackChanged)
    QML_NAMED_ELEMENT(WaylandOutput)
    QML_ADDED_IN_VERSION(1, 0)
public:
    WaylandQuickOutput();
    WaylandQuickOutput(WaylandCompositor *compositor, QWindow *window);

    void update() override;

    bool automaticFrameCallback() const;
    void setAutomaticFrameCallback(bool automatic);

    QQuickItem *pickClickableItem(const QPointF &position);

public Q_SLOTS:
    void updateStarted();

Q_SIGNALS:
    void automaticFrameCallbackChanged();

protected:
    void initialize() override;
    void classBegin() override;
    void componentComplete() override;

private:
    void doFrameCallbacks();

    bool m_updateScheduled = false;
    bool m_automaticFrameCallback = true;
};

} // namespace Compositor

} // namespace Aurora

#endif
