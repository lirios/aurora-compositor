// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QObject>
#include <QtCore/QUrl>

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandClient>

#include "aurorawaylandqtwindowmanager.h"
#include "aurorawaylandqtwindowmanager_p.h"

namespace Aurora {

namespace Compositor {

WaylandQtWindowManagerPrivate::WaylandQtWindowManagerPrivate()
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
        q->openUrl(WaylandClient::fromWlClient(compositor, resource->client()), QUrl(url));
    }
}

WaylandQtWindowManager::WaylandQtWindowManager()
    : WaylandCompositorExtensionTemplate<WaylandQtWindowManager>(*new WaylandQtWindowManagerPrivate())
{
}

WaylandQtWindowManager::WaylandQtWindowManager(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate<WaylandQtWindowManager>(compositor, *new WaylandQtWindowManagerPrivate())
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

#include "moc_aurorawaylandqtwindowmanager.cpp"
