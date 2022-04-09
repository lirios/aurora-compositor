/****************************************************************************
**
** Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef AURORA_COMPOSITOR_WAYLANDOUTPUTMODE_H
#define AURORA_COMPOSITOR_WAYLANDOUTPUTMODE_H

#include <LiriAuroraCompositor/qtwaylandcompositorglobal.h>
#include <QtCore/QSize>

namespace Aurora {

namespace Compositor {

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandOutputMode
{
public:
    explicit WaylandOutputMode();
    WaylandOutputMode(const QSize &size, int refreshRate);
    WaylandOutputMode(const WaylandOutputMode &other);
    ~WaylandOutputMode();

    WaylandOutputMode &operator=(const WaylandOutputMode &other);
    bool operator==(const WaylandOutputMode &other) const;
    bool operator!=(const WaylandOutputMode &other) const;

    bool isValid() const;

    QSize size() const;
    int refreshRate() const;

private:
    class WaylandOutputModePrivate *const d;
    friend class WaylandOutputPrivate;

    void setSize(const QSize &size);
};

} // namespace Compositor

} // namespace Aurora

Q_DECLARE_TYPEINFO(Aurora::Compositor::WaylandOutputMode, Q_MOVABLE_TYPE);

#endif // AURORA_COMPOSITOR_WAYLANDOUTPUTMODE_H
