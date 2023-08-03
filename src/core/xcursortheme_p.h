// SPDX-FileCopyrightText: 2021-2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QImage>
#include <QMap>
#include <QSharedDataPointer>
#include <QString>

#include <chrono>

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

namespace Aurora {

namespace Core {

class XcursorSpritePrivate;
class XcursorThemePrivate;

class XcursorSprite
{
public:
    XcursorSprite();
    XcursorSprite(const XcursorSprite &other);
    XcursorSprite(const QImage &data, const QPoint &hotSpot,
                  const std::chrono::milliseconds &delay);
    ~XcursorSprite();

    XcursorSprite &operator=(const XcursorSprite &other);

    QImage data() const;
    QPoint hotSpot() const;
    std::chrono::milliseconds delay() const;

private:
    QSharedDataPointer<XcursorSpritePrivate> d;
};

class XcursorTheme
{
public:
    XcursorTheme();
    XcursorTheme(const QString &theme, int size, qreal devicePixelRatio);
    XcursorTheme(const XcursorTheme &other);
    ~XcursorTheme();

    XcursorTheme &operator=(const XcursorTheme &other);

    bool operator==(const XcursorTheme &other);
    bool operator!=(const XcursorTheme &other);

    bool isEmpty() const;

    QVector<XcursorSprite> shape(const QByteArray &name) const;

private:
    QSharedDataPointer<XcursorThemePrivate> d;
};

} // namespace Core

} // namespace Aurora
