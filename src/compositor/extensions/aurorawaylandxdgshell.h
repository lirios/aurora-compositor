// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDXDGSHELL_H
#define AURORA_COMPOSITOR_WAYLANDXDGSHELL_H

#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/WaylandResource>
#include <LiriAuroraCompositor/WaylandShell>
#include <LiriAuroraCompositor/WaylandShellSurface>
#if LIRI_FEATURE_aurora_compositor_quick
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>
#endif

#include <QtCore/QRect>

struct wl_resource;

namespace Aurora {

namespace Compositor {

class WaylandClient;
class WaylandOutput;
class WaylandSeat;
class WaylandSurface;
class WaylandSurfaceRole;
class WaylandXdgShellPrivate;
class WaylandXdgSurface;
class WaylandXdgSurfacePrivate;
class WaylandXdgToplevel;
class WaylandXdgToplevelPrivate;
class WaylandXdgPopup;
class WaylandXdgPopupPrivate;
class WaylandXdgPositioner;

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgShell : public WaylandShellTemplate<WaylandXdgShell>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandXdgShell)
public:
    explicit WaylandXdgShell();
    explicit WaylandXdgShell(WaylandCompositor *compositor);
    ~WaylandXdgShell();

    void initialize() override;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

public Q_SLOTS:
    uint ping(Aurora::Compositor::WaylandClient *client);

Q_SIGNALS:
    void xdgSurfaceCreated(Aurora::Compositor::WaylandXdgSurface *xdgSurface);
    void toplevelCreated(Aurora::Compositor::WaylandXdgToplevel *toplevel, Aurora::Compositor::WaylandXdgSurface *xdgSurface);
    void popupCreated(Aurora::Compositor::WaylandXdgPopup *popup, Aurora::Compositor::WaylandXdgSurface *xdgSurface);
    void pong(uint serial);

private Q_SLOTS:
    void handleSeatChanged(Aurora::Compositor::WaylandSeat *newSeat, Aurora::Compositor::WaylandSeat *oldSeat);
    void handleFocusChanged(Aurora::Compositor::WaylandSurface *newSurface, Aurora::Compositor::WaylandSurface *oldSurface);

private:
    QScopedPointer<WaylandXdgShellPrivate> const d_ptr;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgSurface : public WaylandShellSurfaceTemplate<WaylandXdgSurface>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandXdgSurface)
#if LIRI_FEATURE_aurora_compositor_quick
    AURORA_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandXdgSurface)
#endif
    Q_PROPERTY(Aurora::Compositor::WaylandXdgShell *shell READ shell NOTIFY shellChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandSurface *surface READ surface NOTIFY surfaceChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandXdgToplevel *toplevel READ toplevel NOTIFY toplevelCreated)
    Q_PROPERTY(Aurora::Compositor::WaylandXdgPopup *popup READ popup NOTIFY popupCreated)
    Q_PROPERTY(QRect windowGeometry READ windowGeometry NOTIFY windowGeometryChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    Q_MOC_INCLUDE("aurorawaylandsurface.h")
#endif
public:
    explicit WaylandXdgSurface();
    explicit WaylandXdgSurface(WaylandXdgShell* xdgShell, WaylandSurface *surface, const WaylandResource &resource);
    ~WaylandXdgSurface();

    Q_INVOKABLE void initialize(Aurora::Compositor::WaylandXdgShell* xdgShell, Aurora::Compositor::WaylandSurface *surface, const Aurora::Compositor::WaylandResource &resource);

    Qt::WindowType windowType() const override;

    WaylandXdgShell *shell() const override;
    WaylandSurface *surface() const;
    WaylandXdgToplevel *toplevel() const;
    WaylandXdgPopup *popup() const;
    QRect windowGeometry() const;

    static const struct wl_interface *interface();
    static QByteArray interfaceName();
    static WaylandXdgSurface *fromResource(::wl_resource *resource);

#if LIRI_FEATURE_aurora_compositor_quick
    WaylandQuickShellIntegration *createIntegration(WaylandQuickShellSurfaceItem *item) override;
#endif

Q_SIGNALS:
    void surfaceChanged();
    void toplevelCreated();
    void popupCreated();
    void windowGeometryChanged();

private:
    QScopedPointer<WaylandXdgSurfacePrivate> const d_ptr;

    void initialize() override;

private Q_SLOTS:
    void handleSurfaceSizeChanged();
    void handleBufferScaleChanged();
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgToplevel : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandXdgToplevel)
    Q_PROPERTY(Aurora::Compositor::WaylandXdgSurface *xdgSurface READ xdgSurface CONSTANT)
    Q_PROPERTY(Aurora::Compositor::WaylandXdgToplevel *parentToplevel READ parentToplevel NOTIFY parentToplevelChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString appId READ appId NOTIFY appIdChanged)
    Q_PROPERTY(QSize maxSize READ maxSize NOTIFY maxSizeChanged)
    Q_PROPERTY(QSize minSize READ minSize NOTIFY minSizeChanged)
    Q_PROPERTY(QList<int> states READ statesAsInts NOTIFY statesChanged)
    Q_PROPERTY(bool maximized READ maximized NOTIFY maximizedChanged)
    Q_PROPERTY(bool fullscreen READ fullscreen NOTIFY fullscreenChanged)
    Q_PROPERTY(bool resizing READ resizing NOTIFY resizingChanged)
    Q_PROPERTY(bool activated READ activated NOTIFY activatedChanged)
// QDoc fails to parse the property type that includes the keyword 'enum'
#ifndef Q_QDOC
    Q_PROPERTY(enum DecorationMode decorationMode READ decorationMode NOTIFY decorationModeChanged)
#else
    Q_PROPERTY(DecorationMode decorationMode READ decorationMode NOTIFY decorationModeChanged)
#endif
public:
    enum State : uint {
        MaximizedState  = 1,
        FullscreenState = 2,
        ResizingState   = 3,
        ActivatedState  = 4
    };
    Q_ENUM(State)

    enum DecorationMode {
        ClientSideDecoration = 1,
        ServerSideDecoration = 2,
    };
    Q_ENUM(DecorationMode)

    explicit WaylandXdgToplevel(WaylandXdgSurface *xdgSurface, WaylandResource &resource);
    ~WaylandXdgToplevel() override;

    WaylandXdgSurface *xdgSurface() const;
    WaylandXdgToplevel *parentToplevel() const;

    QString title() const;
    QString appId() const;
    QSize maxSize() const;
    QSize minSize() const;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<WaylandXdgToplevel::State> states() const;
#else
    QVector<WaylandXdgToplevel::State> states() const;
#endif
    bool maximized() const;
    bool fullscreen() const;
    bool resizing() const;
    bool activated() const;
    DecorationMode decorationMode() const;

    Q_INVOKABLE QSize sizeForResize(const QSizeF &size, const QPointF &delta, Qt::Edges edges) const;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    uint sendConfigure(const QSize &size, const QList<State> &states);
    Q_INVOKABLE uint sendConfigure(const QSize &size, const QList<int> &states);
#else
    uint sendConfigure(const QSize &size, const QVector<State> &states);
    Q_INVOKABLE uint sendConfigure(const QSize &size, const QVector<int> &states);
#endif
    Q_INVOKABLE void sendClose();
    Q_INVOKABLE uint sendMaximized(const QSize &size);
    Q_INVOKABLE uint sendUnmaximized(const QSize &size = QSize(0, 0));
    Q_INVOKABLE uint sendFullscreen(const QSize &size);
    Q_INVOKABLE uint sendResizing(const QSize &maxSize);

    static WaylandSurfaceRole *role();
    static WaylandXdgToplevel *fromResource(::wl_resource *resource);

Q_SIGNALS:
    void parentToplevelChanged();
    void titleChanged();
    void appIdChanged();
    void maxSizeChanged();
    void minSizeChanged();
    void startMove(Aurora::Compositor::WaylandSeat *seat);
    void startResize(Aurora::Compositor::WaylandSeat *seat, Qt::Edges edges);
    void statesChanged();
    void maximizedChanged();
    void fullscreenChanged();
    void resizingChanged();
    void activatedChanged();

    void showWindowMenu(Aurora::Compositor::WaylandSeat *seat, const QPoint &localSurfacePosition);
    void setMaximized();
    void unsetMaximized();
    void setFullscreen(Aurora::Compositor::WaylandOutput *output);
    void unsetFullscreen();
    void setMinimized();

    void decorationModeChanged();

private:
    QScopedPointer<WaylandXdgToplevelPrivate> const d_ptr;

    QList<int> statesAsInts() const;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandXdgPopup : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandXdgPopup)
    Q_PROPERTY(Aurora::Compositor::WaylandXdgSurface *xdgSurface READ xdgSurface CONSTANT)
    Q_PROPERTY(Aurora::Compositor::WaylandXdgSurface *parentXdgSurface READ parentXdgSurface NOTIFY parentSurfaceChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandSurface *parentSurface READ parentSurface NOTIFY parentSurfaceChanged)
    Q_PROPERTY(QRect configuredGeometry READ configuredGeometry NOTIFY configuredGeometryChanged)

    // Positioner properties
    Q_PROPERTY(QRect anchorRect READ anchorRect CONSTANT)
    Q_PROPERTY(Qt::Edges anchorEdges READ anchorEdges CONSTANT)
    Q_PROPERTY(Qt::Edges gravityEdges READ gravityEdges CONSTANT)
    Q_PROPERTY(Qt::Orientations slideConstraints READ slideConstraints CONSTANT)
    Q_PROPERTY(Qt::Orientations flipConstraints READ flipConstraints CONSTANT)
    Q_PROPERTY(Qt::Orientations resizeConstraints READ resizeConstraints CONSTANT)
    Q_PROPERTY(QPoint offset READ offset CONSTANT)
    Q_PROPERTY(QSize positionerSize READ positionerSize CONSTANT)
    Q_PROPERTY(QPoint unconstrainedPosition READ unconstrainedPosition CONSTANT)
public:
    ~WaylandXdgPopup();

    WaylandXdgSurface *xdgSurface() const;
    WaylandXdgSurface *parentXdgSurface() const;
    WaylandSurface *parentSurface() const;
    QRect configuredGeometry() const;

    // Positioner properties
    QRect anchorRect() const;
    Qt::Edges anchorEdges() const ;
    Qt::Edges gravityEdges() const ;
    Qt::Orientations slideConstraints() const;
    Qt::Orientations flipConstraints() const;
    Qt::Orientations resizeConstraints() const;
    QPoint offset() const;
    QSize positionerSize() const;
    QPoint unconstrainedPosition() const;

    Q_INVOKABLE uint sendConfigure(const QRect &geometry);
    Q_INVOKABLE void sendPopupDone();

    static WaylandSurfaceRole *role();
    static WaylandXdgPopup *fromResource(struct ::wl_resource *resource);

Q_SIGNALS:
    void parentSurfaceChanged();
    void configuredGeometryChanged();

private:
    QScopedPointer<WaylandXdgPopupPrivate> const d_ptr;

    explicit WaylandXdgPopup(WaylandXdgSurface *xdgSurface, WaylandXdgSurface *parentXdgSurface,
                              WaylandXdgPositioner *positioner, WaylandResource &resource);
    friend class WaylandXdgSurfacePrivate;

private slots:
    void handleRedraw();
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDXDGSHELL_H
