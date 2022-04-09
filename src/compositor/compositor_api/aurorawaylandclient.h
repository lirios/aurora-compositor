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

#ifndef AURORA_COMPOSITOR_WAYLANDCLIENT_H
#define AURORA_COMPOSITOR_WAYLANDCLIENT_H

#include <LiriAuroraCompositor/qtwaylandcompositorglobal.h>
#include <LiriAuroraCompositor/qtwaylandqmlinclude.h>

#include <QObject>

#include <signal.h>

struct wl_client;

namespace Aurora {

namespace Compositor {

class WaylandClientPrivate;
class WaylandCompositor;

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandClient : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandClient)

    Q_PROPERTY(WaylandCompositor *compositor READ compositor CONSTANT)
    Q_PROPERTY(qint64 userId READ userId CONSTANT)
    Q_PROPERTY(qint64 groupId READ groupId CONSTANT)
    Q_PROPERTY(qint64 processId READ processId CONSTANT)
    Q_MOC_INCLUDE("aurorawaylandcompositor.h")

    QML_NAMED_ELEMENT(WaylandClient)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("")
public:
    ~WaylandClient() override;

    enum TextInputProtocol {
        NoProtocol = 0,
        QtTextInputMethodV1 = 1,
        TextInputV2 = 2,
        TextInputV4 = 4,

        QtTextInputMethod = QtTextInputMethodV1,
        TextInput = TextInputV2
    };
    Q_DECLARE_FLAGS(TextInputProtocols, TextInputProtocol)

    TextInputProtocols textInputProtocols() const;
    void setTextInputProtocols(TextInputProtocols p);

    static WaylandClient *fromWlClient(WaylandCompositor *compositor, wl_client *wlClient);

    WaylandCompositor *compositor() const;

    wl_client *client() const;

    qint64 userId() const;
    qint64 groupId() const;

    qint64 processId() const;

    Q_INVOKABLE void kill(int signal = SIGTERM);

public Q_SLOTS:
    void close();

private:
    explicit WaylandClient(WaylandCompositor *compositor, wl_client *client);
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDCLIENT_H
