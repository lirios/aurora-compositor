// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDCLIENT_H
#define AURORA_COMPOSITOR_WAYLANDCLIENT_H

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/aurorawaylandqmlinclude.h>

#include <QtCore/QObject>

#include <signal.h>

struct wl_client;

namespace Aurora {

namespace Compositor {

class WaylandClientPrivate;
class WaylandCompositor;

class LIRIAURORACOMPOSITOR_EXPORT WaylandClient : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandClient)

    Q_PROPERTY(Aurora::Compositor::WaylandCompositor *compositor READ compositor CONSTANT)
    Q_PROPERTY(qint64 userId READ userId CONSTANT)
    Q_PROPERTY(qint64 groupId READ groupId CONSTANT)
    Q_PROPERTY(qint64 processId READ processId CONSTANT)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    Q_MOC_INCLUDE("aurorawaylandcompositor.h")
    QML_NAMED_ELEMENT(WaylandClient)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("")
#endif
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
    QScopedPointer<WaylandClientPrivate> const d_ptr;

    explicit WaylandClient(WaylandCompositor *compositor, wl_client *client);
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDCLIENT_H
