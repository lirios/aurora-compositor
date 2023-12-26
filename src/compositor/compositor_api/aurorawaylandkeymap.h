// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QtCore/QObject>
#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/auroraqmlinclude.h>
#if LIRI_FEATURE_aurora_compositor_quick
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>
#endif

namespace Aurora {

namespace Compositor {

class WaylandKeymapPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandKeymap : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandKeymap)
#if LIRI_FEATURE_aurora_compositor_quick
    AURORA_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandKeymap)
#endif
    Q_PROPERTY(QString layout READ layout WRITE setLayout NOTIFY layoutChanged)
    Q_PROPERTY(QString variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(QString options READ options WRITE setOptions NOTIFY optionsChanged)
    Q_PROPERTY(QString rules READ rules WRITE setRules NOTIFY rulesChanged)
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)
    QML_NAMED_ELEMENT(WaylandKeymap)
    QML_ADDED_IN_VERSION(1, 0)
public:
    WaylandKeymap(const QString &layout = QString(), const QString &variant = QString(), const QString &options = QString(),
                   const QString &model = QString(), const QString &rules = QString(), QObject *parent = nullptr);

    QString layout() const;
    void setLayout(const QString &layout);
    QString variant() const;
    void setVariant(const QString &variant);
    QString options() const;
    void setOptions(const QString &options);
    QString rules() const;
    void setRules(const QString &rules);
    QString model() const;
    void setModel(const QString &model);

Q_SIGNALS:
    void layoutChanged();
    void variantChanged();
    void optionsChanged();
    void rulesChanged();
    void modelChanged();
};

} // namespace Compositor

} // namespace Aurora

