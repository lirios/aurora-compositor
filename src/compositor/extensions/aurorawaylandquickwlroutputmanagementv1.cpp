// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aurorawaylandquickwlroutputmanagementv1.h"
#include "aurorawaylandwlroutputmanagementv1_p.h"

namespace Aurora {

namespace Compositor {

/*
 * WaylandQuickWlrOutputHeadV1
 */

WaylandQuickWlrOutputHeadV1::WaylandQuickWlrOutputHeadV1(QObject *parent)
    : WaylandWlrOutputHeadV1(parent)
{
}

QQmlListProperty<WaylandWlrOutputModeV1> WaylandQuickWlrOutputHeadV1::modesList()
{
    return QQmlListProperty<WaylandWlrOutputModeV1>(this, this, modesCount, modesAt);
}

qsizetype WaylandQuickWlrOutputHeadV1::modesCount(QQmlListProperty<WaylandWlrOutputModeV1> *prop)
{
    return static_cast<WaylandQuickWlrOutputHeadV1 *>(prop->object)->modes().size();
}

WaylandWlrOutputModeV1 *
WaylandQuickWlrOutputHeadV1::modesAt(QQmlListProperty<WaylandWlrOutputModeV1> *prop,
                                     qsizetype index)
{
    return static_cast<WaylandQuickWlrOutputHeadV1 *>(prop->object)->modes().at(index);
}

void WaylandQuickWlrOutputHeadV1::componentComplete()
{
    if (!isInitialized()) {
        initialize();

        if (!isInitialized())
            qCWarning(gLcWaylandWlrOutputManagementV1,
                      "Unable to find WlrOutputManagerV1: %p head will not be registered",
                      this);
    }
}

/*
 * WaylandQuickWlrOutputConfigurationV1
 */

WaylandQuickWlrOutputConfigurationV1::WaylandQuickWlrOutputConfigurationV1(QObject *parent)
    : WaylandWlrOutputConfigurationV1(parent)
{
}

QQmlListProperty<WaylandWlrOutputConfigurationHeadV1> WaylandQuickWlrOutputConfigurationV1::enabledHeadsList()
{
    auto countFunc = [](QQmlListProperty<WaylandWlrOutputConfigurationHeadV1> *prop) {
        return static_cast<WaylandQuickWlrOutputConfigurationV1 *>(prop->object)->enabledHeads().size();
    };
    auto atFunc = [](QQmlListProperty<WaylandWlrOutputConfigurationHeadV1> *prop, qsizetype index) {
        return static_cast<WaylandQuickWlrOutputConfigurationV1 *>(prop->object)->enabledHeads().at(index);
    };
    return QQmlListProperty<WaylandWlrOutputConfigurationHeadV1>(this, this, countFunc, atFunc);
}

QQmlListProperty<WaylandWlrOutputHeadV1> WaylandQuickWlrOutputConfigurationV1::disabledHeadsList()
{
    auto countFunc = [](QQmlListProperty<WaylandWlrOutputHeadV1> *prop) {
        return static_cast<WaylandQuickWlrOutputConfigurationV1 *>(prop->object)->disabledHeads().size();
    };
    auto atFunc = [](QQmlListProperty<WaylandWlrOutputHeadV1> *prop, qsizetype index) {
        return static_cast<WaylandQuickWlrOutputConfigurationV1 *>(prop->object)->disabledHeads().at(index);
    };
    return QQmlListProperty<WaylandWlrOutputHeadV1>(this, this, countFunc, atFunc);
}

} // namespace Compositor

} // namespace Aurora
