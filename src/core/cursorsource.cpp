// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "cursorsource.h"

namespace Aurora {

namespace Core {

CursorSource::CursorSource(QObject *parent)
    : QObject(parent)
{
}

QSizeF CursorSource::size() const
{
    return QSizeF(0, 0);
}

QPointF CursorSource::hotSpot() const
{
    return QPointF(0, 0);
}

} // namespace Core

} // namespace Aurora
