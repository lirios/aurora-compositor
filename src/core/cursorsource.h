// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QPointF>
#include <QSizeF>

#include <LiriAuroraCore/liriauroracoreglobal.h>

namespace Aurora {

namespace Core {

class LIRIAURORACORE_EXPORT CursorSource : public QObject
{
    Q_OBJECT
public:
    explicit CursorSource(QObject *parent = nullptr);

    virtual QSizeF size() const;
    virtual QPointF hotSpot() const;

signals:
    void changed();
};

} // namespace Core

} // namespace Aurora
