// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/aurorawaylandcompositor.h>
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
    QML_LIST_PROPERTY_ASSIGN_BEHAVIOR_REPLACE
    Q_PROPERTY(QQmlListProperty<Aurora::Compositor::WaylandOutput> outputs READ outputsListProperty DESIGNABLE false CONSTANT)
    Q_PROPERTY(QQmlListProperty<Aurora::Compositor::WaylandSeat> seats READ seatsListProperty DESIGNABLE false CONSTANT)
public:
    WaylandQuickCompositor(QObject *parent = nullptr);
    void create() override;

    void grabSurface(WaylandSurfaceGrabber *grabber, const WaylandBufferRef &buffer) override;

    QQmlListProperty<WaylandOutput> outputsListProperty();
    QQmlListProperty<WaylandSeat> seatsListProperty();

    static qsizetype outputsCountFunction(QQmlListProperty<WaylandOutput> *list);
    static WaylandOutput *outputsAtFunction(QQmlListProperty<WaylandOutput> *list, qsizetype index);

    static qsizetype seatsCountFunction(QQmlListProperty<WaylandSeat> *list);
    static WaylandSeat *seatsAtFunction(QQmlListProperty<WaylandSeat> *list, qsizetype index);

protected:
    void classBegin() override;
    void componentComplete() override;
};

} // namespace Compositor

} // namespace Aurora

