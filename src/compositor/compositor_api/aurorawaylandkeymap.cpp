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

#include "aurorawaylandkeymap.h"
#include "aurorawaylandkeymap_p.h"

namespace Aurora {

namespace Compositor {

WaylandKeymap::WaylandKeymap(const QString &layout, const QString &variant, const QString &options, const QString &model, const QString &rules, QObject *parent)
    : QObject(*new WaylandKeymapPrivate(layout, variant, options, model, rules), parent)
{
}

QString WaylandKeymap::layout() const {
    Q_D(const WaylandKeymap);
    return d->m_layout;
}

void WaylandKeymap::setLayout(const QString &layout)
{
    Q_D(WaylandKeymap);
    if (d->m_layout == layout)
        return;
    d->m_layout = layout;
    emit layoutChanged();
}

QString WaylandKeymap::variant() const
{
    Q_D(const WaylandKeymap);
    return d->m_variant;
}

void WaylandKeymap::setVariant(const QString &variant)
{
    Q_D(WaylandKeymap);
    if (d->m_variant == variant)
        return;
    d->m_variant = variant;
    emit variantChanged();
}

QString WaylandKeymap::options() const {
    Q_D(const WaylandKeymap);
    return d->m_options;
}

void WaylandKeymap::setOptions(const QString &options)
{
    Q_D(WaylandKeymap);
    if (d->m_options == options)
        return;
    d->m_options = options;
    emit optionsChanged();
}

QString WaylandKeymap::rules() const {
    Q_D(const WaylandKeymap);
    return d->m_rules;
}

void WaylandKeymap::setRules(const QString &rules)
{
    Q_D(WaylandKeymap);
    if (d->m_rules == rules)
        return;
    d->m_rules = rules;
    emit rulesChanged();
}

QString WaylandKeymap::model() const {
    Q_D(const WaylandKeymap);
    return d->m_model;
}

void WaylandKeymap::setModel(const QString &model)
{
    Q_D(WaylandKeymap);
    if (d->m_model == model)
        return;
    d->m_model = model;
    emit modelChanged();
}

WaylandKeymapPrivate::WaylandKeymapPrivate(const QString &layout, const QString &variant,
                                             const QString &options, const QString &model,
                                             const QString &rules)
    : m_layout(layout)
    , m_variant(variant)
    , m_options(options)
    , m_rules(rules)
    , m_model(model)
{
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandkeymap.cpp"
