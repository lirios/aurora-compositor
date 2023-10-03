// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

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

#include <QDBusUnixFileDescriptor>

#include "session.h"

namespace Aurora {

namespace Platform {

class LogindSession : public Session
{
    Q_OBJECT
public:
    ~LogindSession() override;

    QString name() const override;

    Capabilities capabilities() const override;

    bool isActive() const override;

    QString seat() const override;
    uint terminal() const override;

    int openRestricted(const QString &fileName) override;
    void closeRestricted(int fileDescriptor) override;

    void switchTo(uint terminal) override;

    static LogindSession *create(QObject *parent = nullptr);

private:
    explicit LogindSession(const QString &sessionPath, QObject *parent = nullptr);

    bool initialize();
    void updateActive(bool active);

    QString m_sessionPath;
    QString m_seatId;
    QString m_seatPath;
    uint m_terminal = 0;
    bool m_isActive = false;

private slots:
    void handleResumeDevice(uint major, uint minor, QDBusUnixFileDescriptor fileDescriptor);
    void handlePauseDevice(uint major, uint minor, const QString &type);
    void handlePropertiesChanged(const QString &interfaceName, const QVariantMap &properties);
    void handlePrepareForSleep(bool sleep);
};

} // namespace Platform

} // namespace Aurora