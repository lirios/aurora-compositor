// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/WaylandClient>

#include <QtCore/QUrl>

namespace Aurora {

namespace Compositor {

class WaylandQtWindowManagerPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandQtWindowManager : public WaylandCompositorExtensionTemplate<WaylandQtWindowManager>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandQtWindowManager)
    Q_PROPERTY(bool showIsFullScreen READ showIsFullScreen WRITE setShowIsFullScreen NOTIFY showIsFullScreenChanged)
public:
    WaylandQtWindowManager();
    explicit WaylandQtWindowManager(WaylandCompositor *compositor);

    bool showIsFullScreen() const;
    void setShowIsFullScreen(bool value);

    void sendQuitMessage(WaylandClient *client);

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void showIsFullScreenChanged();
    void openUrl(Aurora::Compositor::WaylandClient *client, const QUrl &url);

};

} // namespace Compositor

} // namespace Aurora

