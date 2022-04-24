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

#include <QtCore/QObject>
#include <QtCore/QUrl>

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandClient>

#include "aurorawaylandqtwindowmanager.h"
#include "aurorawaylandqtwindowmanager_p.h"

namespace Aurora {

namespace Compositor {

WaylandQtWindowManagerPrivate::WaylandQtWindowManagerPrivate(WaylandQtWindowManager *self)
    : WaylandCompositorExtensionPrivate(self)
{
}

void WaylandQtWindowManagerPrivate::windowmanager_bind_resource(Resource *resource)
{
    send_hints(resource->handle, static_cast<int32_t>(showIsFullScreen));
}

void WaylandQtWindowManagerPrivate::windowmanager_destroy_resource(Resource *resource)
{
    urls.remove(resource);
}

void WaylandQtWindowManagerPrivate::windowmanager_open_url(Resource *resource, uint32_t remaining, const QString &newUrl)
{
    Q_Q(WaylandQtWindowManager);

    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(q->extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find WaylandCompositor from WaylandQtWindowManager::windowmanager_open_url()";
        return;
    }

    QString url = urls.value(resource, QString());

    url.append(newUrl);

    if (remaining)
        urls.insert(resource, url);
    else {
        urls.remove(resource);
        emit q->openUrl(WaylandClient::fromWlClient(compositor, resource->client()), QUrl(url));
    }
}

WaylandQtWindowManager::WaylandQtWindowManager()
    : WaylandCompositorExtensionTemplate<WaylandQtWindowManager>()
    , d_ptr(new WaylandQtWindowManagerPrivate(this))
{
}

WaylandQtWindowManager::WaylandQtWindowManager(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate<WaylandQtWindowManager>(compositor)
    , d_ptr(new WaylandQtWindowManagerPrivate(this))
{
}

WaylandQtWindowManager::~WaylandQtWindowManager()
{
}

bool WaylandQtWindowManager::showIsFullScreen() const
{
    Q_D(const WaylandQtWindowManager);
    return d->showIsFullScreen;
}

void WaylandQtWindowManager::setShowIsFullScreen(bool value)
{
    Q_D(WaylandQtWindowManager);

    if (d->showIsFullScreen == value)
        return;

    d->showIsFullScreen = value;
    const auto resMap = d->resourceMap();
    for (WaylandQtWindowManagerPrivate::Resource *resource : resMap) {
        d->send_hints(resource->handle, static_cast<int32_t>(d->showIsFullScreen));
    }
    Q_EMIT showIsFullScreenChanged();
}

void WaylandQtWindowManager::sendQuitMessage(WaylandClient *client)
{
    Q_D(WaylandQtWindowManager);
    WaylandQtWindowManagerPrivate::Resource *resource = d->resourceMap().value(client->client());

    if (resource)
        d->send_quit(resource->handle);
}

void WaylandQtWindowManager::initialize()
{
    Q_D(WaylandQtWindowManager);

    WaylandCompositorExtensionTemplate::initialize();
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find WaylandCompositor when initializing WaylandQtWindowManager";
        return;
    }
    d->init(compositor->display(), 1);
}

const struct wl_interface *WaylandQtWindowManager::interface()
{
    return WaylandQtWindowManagerPrivate::interface();
}

QByteArray WaylandQtWindowManager::interfaceName()
{
    return WaylandQtWindowManagerPrivate::interfaceName();
}

} // namespace Compositor

} // namespace Aurora
