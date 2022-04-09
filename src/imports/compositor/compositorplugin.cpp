/****************************************************************************
**
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

#include <QtCore/QDir>

#include <QtQml/qqmlextensionplugin.h>

#include <QtQuick/QQuickItem>

#include <LiriAuroraCompositor/WaylandQuickCompositor>
#include <LiriAuroraCompositor/WaylandQuickItem>
#include <LiriAuroraCompositor/private/aurorawaylandquickhardwarelayer_p.h>
#include <LiriAuroraCompositor/WaylandQuickSurface>
#include <LiriAuroraCompositor/WaylandClient>
#include <LiriAuroraCompositor/WaylandQuickOutput>
#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/WaylandQuickExtension>
#include <LiriAuroraCompositor/WaylandSeat>
#include <LiriAuroraCompositor/WaylandDrag>
#include <LiriAuroraCompositor/WaylandKeymap>
#include <LiriAuroraCompositor/WaylandQuickShellSurfaceItem>
#include <LiriAuroraCompositor/WaylandResource>
#include <LiriAuroraCompositor/WaylandShell>
#include <LiriAuroraCompositor/WaylandShellSurface>

#include <LiriAuroraCompositor/WaylandQtWindowManager>
#include <LiriAuroraCompositor/WaylandTextInputManager>
#include <LiriAuroraCompositor/WaylandIdleInhibitManagerV1>
#include <LiriAuroraCompositor/private/aurorawaylandmousetracker_p.h>

namespace Aurora {

namespace Compositor {

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CONTAINER_CLASS(WaylandQuickCompositor)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandQtWindowManager)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandIdleInhibitManagerV1)
Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(WaylandTextInputManager)

class QmlUrlResolver
{
public:
    QmlUrlResolver(bool useResource, const QDir &qmlDir, const QString &qrcPath)
        : m_useResource(useResource)
        , m_qmlDir(qmlDir)
        , m_qrcPath(qrcPath)
    { }

    QUrl get(const QString &fileName)
    {
        return m_useResource ? QUrl(m_qrcPath + fileName) :
            QUrl::fromLocalFile(m_qmlDir.filePath(fileName));
    }
private:
    bool m_useResource;
    const QDir m_qmlDir;
    const QString m_qrcPath;
};

class AuroraWaylandCompositorPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Aurora.Compositor"));
        defineModule(uri);

        bool useResource = true;
        QDir qmlDir(baseUrl().toLocalFile());
        if (qmlDir.exists(QStringLiteral("WaylandCursorItem.qml")))
            useResource = false;

        QmlUrlResolver resolver(useResource, qmlDir, QStringLiteral("qrc:/Aurora/Compositor/"));

        qmlRegisterType(resolver.get(QStringLiteral("WaylandOutputWindow.qml")), uri, 1, 0, "WaylandOutputWindow");
        qmlRegisterType(resolver.get(QStringLiteral("WaylandCursorItem.qml")), uri, 1, 0, "WaylandCursorItem");
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, 1, 0);

        qmlRegisterType<WaylandQuickCompositorQuickExtensionContainer>(uri, 1, 0, "WaylandCompositor");
        qmlRegisterType<WaylandQuickItem>(uri, 1, 0, "WaylandQuickItem");
#if QT_CONFIG(opengl)
        qmlRegisterType<WaylandQuickHardwareLayer>(uri, 1, 0, "WaylandHardwareLayer");
#endif
        qmlRegisterType<WaylandMouseTracker>(uri, 1, 0, "WaylandMouseTracker");
        qmlRegisterType<WaylandQuickOutput>(uri, 1, 0, "WaylandOutput");
        qmlRegisterType<WaylandQuickSurface>(uri, 1, 0, "WaylandSurface");
        qmlRegisterType<WaylandKeymap>(uri, 1, 0, "WaylandKeymap");

        qmlRegisterUncreatableType<WaylandCompositorExtension>(uri, 1, 0, "WaylandExtension", QObject::tr("Cannot create instance of WaylandExtension"));
        qmlRegisterUncreatableType<WaylandClient>(uri, 1, 0, "WaylandClient", QObject::tr("Cannot create instance of WaylandClient"));
        qmlRegisterUncreatableType<WaylandOutput>(uri, 1, 0, "WaylandOutputBase", QObject::tr("Cannot create instance of WaylandOutputBase, use WaylandOutput instead"));
        qmlRegisterUncreatableType<WaylandSeat>(uri, 1, 0, "WaylandSeat", QObject::tr("Cannot create instance of WaylandSeat"));
        qmlRegisterUncreatableType<WaylandDrag>(uri, 1, 0, "WaylandDrag", QObject::tr("Cannot create instance of WaylandDrag"));
        qmlRegisterUncreatableType<WaylandCompositor>(uri, 1, 0, "WaylandCompositorBase", QObject::tr("Cannot create instance of WaylandCompositorBase, use WaylandCompositor instead"));
        qmlRegisterUncreatableType<WaylandSurface>(uri, 1, 0, "WaylandSurfaceBase", QObject::tr("Cannot create instance of WaylandSurfaceBase, use WaylandSurface instead"));
        qmlRegisterUncreatableType<WaylandShell>(uri, 1, 0, "Shell", QObject::tr("Cannot create instance of Shell"));
        qmlRegisterUncreatableType<WaylandShellSurface>(uri, 1, 0, "ShellSurface", QObject::tr("Cannot create instance of ShellSurface"));
        qmlRegisterType<WaylandQuickShellSurfaceItem>(uri, 1, 0, "ShellSurfaceItem");
        qmlRegisterUncreatableType<WaylandResource>(uri, 1, 0, "WaylandResource", QObject::tr("Cannot create instance of WaylandResource"));

        qmlRegisterType<WaylandQtWindowManagerQuickExtension>(uri, 1, 0, "QtWindowManager");

        qmlRegisterType<WaylandTextInputManagerQuickExtension>(uri, 1, 0, "TextInputManager");

        qmlRegisterType<WaylandIdleInhibitManagerV1QuickExtension>(uri, 1, 14, "IdleInhibitManagerV1");
    }
};

} // namespace Compositor

} // namespace Aurora

#include "compositorplugin.moc"
