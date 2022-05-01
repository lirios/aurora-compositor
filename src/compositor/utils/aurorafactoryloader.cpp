/****************************************************************************
**
** Copyright (C) 2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2018 Intel Corporation.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QPluginLoader>
#include <QtCore/QJsonArray>

#include "aurorafactoryloader_p.h"

namespace Aurora {

namespace Compositor {

namespace Internal {

FactoryLoader::FactoryLoader(const char *iid, const QString &suffix, Qt::CaseSensitivity cs)
    : m_cs(cs)
{
    // Find all the plugins with the specified IID
    const auto paths = QCoreApplication::libraryPaths();
    for (const QString &path : paths) {
        QDir pluginsPath(QDir(path).absoluteFilePath(suffix));

        const auto files = pluginsPath.entryList(QDir::Files);
        for (const QString &fileName : files) {
            QPluginLoader loader(pluginsPath.absoluteFilePath(fileName));

            if (loader.metaData().value(QLatin1String("IID")).toString() != QLatin1String(iid))
                continue;

            m_metaDatas.append(loader.metaData());
            m_instances.append(loader.instance());
        }
    }

    // Collect the keys
    for (int i = 0; i < m_metaDatas.size(); ++i) {
        const QJsonObject metaData = m_metaDatas.at(i).value(QLatin1String("MetaData")).toObject();
        const QJsonArray keys = metaData.value(QLatin1String("Keys")).toArray();
        for (const auto &key : keys)
            m_keys.insert(i, key.toString());
    }
}

QList<QJsonObject> FactoryLoader::metaData() const
{
    return m_metaDatas;
}

QMultiMap<int, QString> FactoryLoader::keyMap() const
{
    return m_keys;
}

QObject *FactoryLoader::instance(int index) const
{
    return m_instances.at(index);
}

int FactoryLoader::indexOf(const QString &lookupKey) const
{
    for (int i = 0; i < m_metaDatas.size(); ++i) {
        const QJsonObject metaData = m_metaDatas.at(i).value(QLatin1String("MetaData")).toObject();
        const QJsonArray keys = metaData.value(QLatin1String("Keys")).toArray();
        for (const auto &key : keys) {
            if (!key.toString().compare(lookupKey, m_cs))
                return i;
        }
    }

    return -1;
}

} // namespace Internal

} // namespace Compositor

} // namespace Aurora
