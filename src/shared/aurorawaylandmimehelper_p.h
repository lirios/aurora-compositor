// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDMIMEHELPER_H
#define AURORA_COMPOSITOR_WAYLANDMIMEHELPER_H

#include <QString>
#include <QByteArray>
#include <QMimeData>

namespace Aurora {

namespace Compositor {

class WaylandMimeHelper
{
public:
    static QByteArray getByteArray(QMimeData *mimeData, const QString &mimeType);
};

} // namespace Compositor

} // namespace Aurora

#endif
