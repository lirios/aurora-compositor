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

#ifndef AURORA_COMPOSITOR_WAYLANDKEYMAP_H
#define AURORA_COMPOSITOR_WAYLANDKEYMAP_H

#include <QtCore/QObject>
#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/aurorawaylandqmlinclude.h>
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>

namespace Aurora {

namespace Compositor {

class WaylandKeymapPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandKeymap : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandKeymap)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandKeymap)
    Q_PROPERTY(QString layout READ layout WRITE setLayout NOTIFY layoutChanged)
    Q_PROPERTY(QString variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(QString options READ options WRITE setOptions NOTIFY optionsChanged)
    Q_PROPERTY(QString rules READ rules WRITE setRules NOTIFY rulesChanged)
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(WaylandKeymap)
    QML_ADDED_IN_VERSION(1, 0)
#endif
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

#endif //QWAYLANDKEYMAP_H
