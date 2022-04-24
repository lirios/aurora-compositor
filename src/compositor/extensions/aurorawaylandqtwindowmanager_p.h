/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef AURORA_COMPOSITOR_WAYLANDQTWINDOWMANAGER_P_H
#define AURORA_COMPOSITOR_WAYLANDQTWINDOWMANAGER_P_H

#include <QtCore/QMap>

#include <LiriAuroraCompositor/WaylandQtWindowManager>
#include <LiriAuroraCompositor/private/aurorawaylandcompositorextension_p.h>
#include <LiriAuroraCompositor/private/aurora-server-qt-windowmanager.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Aurora API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandQtWindowManagerPrivate
        : public WaylandCompositorExtensionPrivate
        , public PrivateServer::qt_windowmanager
{
    Q_DECLARE_PUBLIC(WaylandQtWindowManager)
public:
    WaylandQtWindowManagerPrivate(WaylandQtWindowManager *self);

protected:
    void windowmanager_bind_resource(Resource *resource) override;
    void windowmanager_destroy_resource(Resource *resource) override;
    void windowmanager_open_url(Resource *resource, uint32_t remaining, const QString &url) override;

private:
    bool showIsFullScreen = false;
    QMap<Resource*, QString> urls;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDQTWINDOWMANAGER_P_H
