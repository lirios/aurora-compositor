// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
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

#include "session.h"

namespace Aurora {

namespace Platform {

class NoopSession : public Session
{
    Q_OBJECT
public:
    ~NoopSession() override;

    QString name() const override;

    Capabilities capabilities() const override;

    bool isActive() const override;

    QString seat() const override;
    uint terminal() const override;

    int openRestricted(const QString &fileName) override;
    void closeRestricted(int fileDescriptor) override;

    void switchTo(uint terminal) override;

    static NoopSession *create(QObject *parent = nullptr);

private:
    explicit NoopSession(QObject *parent = nullptr);
};

} // namespace Platform

} // namespace Aurora