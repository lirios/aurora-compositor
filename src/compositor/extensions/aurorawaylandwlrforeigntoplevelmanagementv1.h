// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AURORA_COMPOSITOR_WAYLANDWLRFOREIGNTOPLEVELMANAGEMENTV1_H
#define AURORA_COMPOSITOR_WAYLANDWLRFOREIGNTOPLEVELMANAGEMENTV1_H

#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>

namespace Aurora {

namespace Compositor {

class WaylandClient;
class WaylandCompositor;
class WaylandOutput;
class WaylandSeat;
class WaylandSurface;
class WaylandWlrForeignToplevelHandleV1;
class WaylandWlrForeignToplevelManagerV1Private;
class WaylandWlrForeignToplevelHandleV1Private;

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrForeignToplevelManagerV1
        : public WaylandCompositorExtensionTemplate<WaylandWlrForeignToplevelManagerV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrForeignToplevelManagerV1)
public:
    WaylandWlrForeignToplevelManagerV1();
    WaylandWlrForeignToplevelManagerV1(WaylandCompositor *compositor);
    ~WaylandWlrForeignToplevelManagerV1();

    void initialize() override;

    static const wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void handleAdded(Aurora::Compositor::WaylandWlrForeignToplevelHandleV1 *handle);
    void clientStopped(Aurora::Compositor::WaylandClient *client);

private:
    QScopedPointer<WaylandWlrForeignToplevelManagerV1Private> const d_ptr;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrForeignToplevelHandleV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrForeignToplevelHandleV1)
    Q_PROPERTY(Aurora::Compositor::WaylandCompositor *compositor READ compositor WRITE setCompositor NOTIFY compositorChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandWlrForeignToplevelManagerV1 *manager READ manager WRITE setManager NOTIFY managerChanged)
    Q_PROPERTY(bool maximized READ isMaximized WRITE setMaximized NOTIFY maximizedChanged)
    Q_PROPERTY(bool minimized READ isMinimized WRITE setMinimized NOTIFY minimizedChanged)
    Q_PROPERTY(bool activated READ isActivated WRITE setActivated NOTIFY activatedChanged)
    Q_PROPERTY(bool fullscreen READ isFullscreen WRITE setFullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString appId READ appId WRITE setAppId NOTIFY appIdChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandSurface *rectangleSurface READ rectangleSurface NOTIFY rectangleChanged)
    Q_PROPERTY(qint32 rectangleX READ rectangleX NOTIFY rectangleChanged)
    Q_PROPERTY(qint32 rectangleY READ rectangleY NOTIFY rectangleChanged)
    Q_PROPERTY(qint32 rectangleWidth READ rectangleWidth NOTIFY rectangleChanged)
    Q_PROPERTY(qint32 rectangleHeight READ rectangleHeight NOTIFY rectangleChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandWlrForeignToplevelHandleV1 *parent READ parent WRITE setParent NOTIFY parentChanged)
public:
    explicit WaylandWlrForeignToplevelHandleV1(QObject *parent = nullptr);
    ~WaylandWlrForeignToplevelHandleV1();

    bool isInitialized() const;

    WaylandCompositor *compositor() const;
    void setCompositor(WaylandCompositor *compositor);

    WaylandWlrForeignToplevelManagerV1 *manager() const;
    void setManager(WaylandWlrForeignToplevelManagerV1 *manager);

    bool isMaximized() const;
    void setMaximized(bool maximized);

    bool isMinimized() const;
    void setMinimized(bool minimized);

    bool isFullscreen() const;
    void setFullscreen(bool fullscreen);

    bool isActivated() const;
    void setActivated(bool activated);

    QString title() const;
    void setTitle(const QString &title);

    QString appId() const;
    void setAppId(const QString &appId);

    WaylandSurface *rectangleSurface() const;
    QRect rectangle() const;
    qint32 rectangleX() const;
    qint32 rectangleY() const;
    qint32 rectangleWidth() const;
    qint32 rectangleHeight() const;

    WaylandWlrForeignToplevelHandleV1 *parent() const;
    void setParent(WaylandWlrForeignToplevelHandleV1 *parentHandle);

    Q_INVOKABLE void sendOutputEnter(Aurora::Compositor::WaylandOutput *output);
    Q_INVOKABLE void sendOutputLeave(Aurora::Compositor::WaylandOutput *output);
    Q_INVOKABLE void sendClosed();

public Q_SLOTS:
    void initialize();

Q_SIGNALS:
    void compositorChanged();
    void managerChanged();
    void titleChanged();
    void appIdChanged();
    void maximizedChanged();
    void minimizedChanged();
    void fullscreenChanged();
    void activatedChanged();
    void maximizeRequested();
    void unmaximizeRequested();
    void minimizeRequested();
    void unminimizeRequested();
    void fullscreenRequested(Aurora::Compositor::WaylandOutput *output);
    void unfullscreenRequested();
    void activateRequested(Aurora::Compositor::WaylandSeat *seat);
    void closeRequested();
    void rectangleChanged();
    void parentChanged();

private:
    QScopedPointer<WaylandWlrForeignToplevelHandleV1Private> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDWLRFOREIGNTOPLEVELMANAGEMENTV1_H
