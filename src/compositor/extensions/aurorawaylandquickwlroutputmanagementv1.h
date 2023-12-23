// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QQmlComponent>
#include <QQmlParserStatus>
#include <QQmlListProperty>

#include <LiriAuroraCompositor/WaylandWlrOutputManagerV1>

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickWlrOutputHeadV1
        : public WaylandWlrOutputHeadV1, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Aurora::Compositor::WaylandWlrOutputModeV1> modes READ modesList NOTIFY modesChanged)
    AURORA_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandQuickWlrOutputHeadV1)
    Q_INTERFACES(QQmlParserStatus)
public:
    explicit WaylandQuickWlrOutputHeadV1(QObject *parent = nullptr);

    QQmlListProperty<WaylandWlrOutputModeV1> modesList();

protected:
    void classBegin() override {}
    void componentComplete() override;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickWlrOutputConfigurationV1
        : public WaylandWlrOutputConfigurationV1
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Aurora::Compositor::WaylandWlrOutputConfigurationHeadV1> enabledHeads READ enabledHeadsList NOTIFY enabledHeadsChanged)
    Q_PROPERTY(QQmlListProperty<Aurora::Compositor::WaylandWlrOutputHeadV1> disabledHeads READ disabledHeadsList NOTIFY disabledHeadsChanged)
public:
    explicit WaylandQuickWlrOutputConfigurationV1(QObject *parent = nullptr);

    QQmlListProperty<WaylandWlrOutputConfigurationHeadV1> enabledHeadsList();
    QQmlListProperty<WaylandWlrOutputHeadV1> disabledHeadsList();
};

} // namespace Compositor

} // namespace Aurora

QML_DECLARE_TYPE(Aurora::Compositor::WaylandWlrOutputConfigurationHeadV1)
QML_DECLARE_TYPE(Aurora::Compositor::WaylandQuickWlrOutputHeadV1)
QML_DECLARE_TYPE(Aurora::Compositor::WaylandWlrOutputModeV1)

