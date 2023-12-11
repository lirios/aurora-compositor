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

#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonObject>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Aurora API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>

namespace Aurora {

namespace Compositor {

namespace Internal {

class LIRIAURORACOMPOSITOR_EXPORT FactoryLoader
{
public:
    explicit FactoryLoader(const char *iid, const QString &suffix, Qt::CaseSensitivity cs);

    QList<QJsonObject> metaData() const;
    QMultiMap<int, QString> keyMap() const;

    QObject *instance(int index) const;
    int indexOf(const QString &lookupKey) const;

private:
    QList<QJsonObject> m_metaDatas;
    QMultiMap<int, QString> m_keys;
    QObjectList m_instances;
    Qt::CaseSensitivity m_cs;
};

template <class PluginInterface, class FactoryInterface, typename ...Args>
PluginInterface *auroraLoadPlugin(const FactoryLoader *loader, const QString &key, Args &&...args)
{
    const int index = loader->indexOf(key);
    if (index != -1) {
        QObject *factoryObject = loader->instance(index);
        if (FactoryInterface *factory = qobject_cast<FactoryInterface *>(factoryObject))
            if (PluginInterface *result = factory->create(key, std::forward<Args>(args)...))
                return result;
    }
    return nullptr;
}

} // namespace Internal

} // namespace Compositor

} // namespace Aurora

