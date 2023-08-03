// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <LiriAuroraCore/CursorSource>

namespace Aurora {

namespace Core {

class ShapeCursorSourcePrivate;

class LIRIAURORACORE_EXPORT ShapeCursorSource : public CursorSource
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ShapeCursorSource)
public:
    explicit ShapeCursorSource(QObject *parent = nullptr);
    ~ShapeCursorSource();

    QSizeF size() const override;
    QPointF hotSpot() const override;

    QImage image() const;

    QByteArray shape() const;
    void setShape(const QByteArray &shape);
    void setShape(Qt::CursorShape shape);

    void loadTheme(const QString &themeName, int size, qreal devicePixelRatio);

protected:
    QScopedPointer<ShapeCursorSourcePrivate> const d_ptr;

private:
    Q_PRIVATE_SLOT(d_func(), void selectNextSprite())
};

} // namespace Core

} // namespace Aurora
