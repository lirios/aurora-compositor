// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

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
    WaylandQtWindowManagerPrivate();

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

