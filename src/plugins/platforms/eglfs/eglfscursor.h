// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPointer>

#include <QtGui/qpa/qplatformcursor.h>
#include <QtGui/qpa/qplatformscreen.h>

class EglFSScreen;

class EglFSCursor : public QPlatformCursor
{
    Q_OBJECT
public:
    explicit EglFSCursor(QPlatformScreen *screen);
    ~EglFSCursor();

#ifndef QT_NO_CURSOR
    void changeCursor(QCursor *cursor, QWindow *widget) override;
#endif

private:
    QPointer<EglFSScreen> m_screen;
};
