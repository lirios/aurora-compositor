// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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

#pragma once

#include <QOpenGLFunctions>
#include <QQuickImageProvider>
#include <QtQuick/QSGTexture>
#include <QScopedPointer>
#include <QHash>

#include <QtWaylandClient/private/qwaylandserverbufferintegration_p.h>

namespace Aurora {

namespace Compositor {

class TextureSharingExtension;

class SharedTextureRegistry : public QObject
{
    Q_OBJECT
public:
    SharedTextureRegistry();
    ~SharedTextureRegistry() override;

    const QtWaylandClient::WaylandServerBuffer *bufferForId(const QString &id) const;
    void requestBuffer(const QString &id);
    void abandonBuffer(const QString &id);

    static bool preinitialize();

public Q_SLOTS:
    void receiveBuffer(QtWaylandClient::WaylandServerBuffer *buffer, const QString &id);

Q_SIGNALS:
    void replyReceived(const QString &id);

private Q_SLOTS:
    void handleExtensionActive();

private:
    TextureSharingExtension *m_extension = nullptr;
    QHash<QString, QtWaylandClient::WaylandServerBuffer *> m_buffers;
    QStringList m_pendingBuffers;
};

class SharedTextureProvider : public QQuickAsyncImageProvider
{
public:
    SharedTextureProvider();
    ~SharedTextureProvider() override;

    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;

private:
    SharedTextureRegistry *m_registry = nullptr;
    bool m_sharingAvailable = false;
};

} // namespace Compositor

} // namespace Aurora

