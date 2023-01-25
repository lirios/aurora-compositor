// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDQUICKCOMPOSITOR_H
#define AURORA_COMPOSITOR_WAYLANDQUICKCOMPOSITOR_H

#include <LiriAuroraCompositor/aurorawaylandcompositor.h>
#include <QtQml/QQmlListProperty>
#include <QtQml/QQmlParserStatus>

class QQuickWindow;

namespace Aurora {

namespace Compositor {

class WaylandQuickCompositorPrivate;
class WaylandView;

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickCompositor : public WaylandCompositor, public QQmlParserStatus
{
    Q_INTERFACES(QQmlParserStatus)
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Aurora::Compositor::WaylandOutput> outputs READ outputsListProperty DESIGNABLE false CONSTANT)
    Q_PROPERTY(QQmlListProperty<Aurora::Compositor::WaylandSeat> seats READ seatsListProperty DESIGNABLE false CONSTANT)
    Q_INTERFACES(QQmlParserStatus)
public:
    WaylandQuickCompositor(QObject *parent = nullptr);
    ~WaylandQuickCompositor();

    void create() override;

    void grabSurface(WaylandSurfaceGrabber *grabber, const WaylandBufferRef &buffer) override;

    QQmlListProperty<WaylandOutput> outputsListProperty();
    QQmlListProperty<WaylandSeat> seatsListProperty();

    static int outputsCountFunction(QQmlListProperty<WaylandOutput> *list);
    static WaylandOutput *outputsAtFunction(QQmlListProperty<WaylandOutput> *list, int index);

    static int seatsCountFunction(QQmlListProperty<WaylandSeat> *list);
    static WaylandSeat *seatsAtFunction(QQmlListProperty<WaylandSeat> *list, int index);

protected:
    void classBegin() override;
    void componentComplete() override;

private:
    QScopedPointer<WaylandQuickCompositorPrivate> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

#endif
