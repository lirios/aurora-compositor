// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandkeymap.h"
#include "aurorawaylandkeymap_p.h"

namespace Aurora {

namespace Compositor {

WaylandKeymap::WaylandKeymap(const QString &layout, const QString &variant, const QString &options, const QString &model, const QString &rules, QObject *parent)
    : QObject(parent)
    , d_ptr(new WaylandKeymapPrivate(this, layout, variant, options, model, rules))
{
}

WaylandKeymap::~WaylandKeymap()
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

WaylandKeymapPrivate::WaylandKeymapPrivate(WaylandKeymap *self,
                                           const QString &layout, const QString &variant,
                                           const QString &options, const QString &model,
                                           const QString &rules)
    : q_ptr(self)
    , m_layout(layout)
    , m_variant(variant)
    , m_options(options)
    , m_rules(rules)
    , m_model(model)
{
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandkeymap.cpp"
