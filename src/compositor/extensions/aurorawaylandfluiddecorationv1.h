// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPointer>

#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/WaylandResource>
#include <LiriAuroraCompositor/WaylandSurface>

struct wl_client;

namespace Aurora {

namespace Compositor {

class WaylandFluidDecorationManagerV1Private;
class WaylandFluidDecorationV1;
class WaylandFluidDecorationV1Private;

class LIRIAURORACOMPOSITOR_EXPORT WaylandFluidDecorationManagerV1
        : public WaylandCompositorExtensionTemplate<WaylandFluidDecorationManagerV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandFluidDecorationManagerV1)
public:
    WaylandFluidDecorationManagerV1();
    WaylandFluidDecorationManagerV1(WaylandCompositor *compositor);
    ~WaylandFluidDecorationManagerV1();

    void initialize() override;

    void unregisterDecoration(WaylandFluidDecorationV1 *decoration);

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void decorationCreated(Aurora::Compositor::WaylandFluidDecorationV1 *decoration);

private:
    QScopedPointer<WaylandFluidDecorationManagerV1Private> const d_ptr;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandFluidDecorationV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandFluidDecorationV1)
    Q_PROPERTY(Aurora::Compositor::WaylandSurface *surface READ surface CONSTANT)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor NOTIFY foregroundColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY backgroundColorChanged)
public:
    ~WaylandFluidDecorationV1();

    WaylandSurface *surface() const;
    wl_resource *surfaceResource() const;

    QColor foregroundColor() const;
    QColor backgroundColor() const;

Q_SIGNALS:
    void foregroundColorChanged(const QColor &color);
    void backgroundColorChanged(const QColor &color);

private:
    QScopedPointer<WaylandFluidDecorationV1Private> const d_ptr;

    explicit WaylandFluidDecorationV1(WaylandFluidDecorationManagerV1 *manager,
                                      WaylandSurface *surface,
                                      wl_client *client,
                                      quint32 id, quint32 version);

    friend class WaylandFluidDecorationManagerV1Private;
};

} // namespace Compositor

} // namespace Aurora

