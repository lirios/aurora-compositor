// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "session_noop_p.h"

namespace Aurora {

namespace Platform {

NoopSession::NoopSession(QObject *parent)
    : Session(parent)
{
}

NoopSession::~NoopSession()
{
}

QString NoopSession::name() const
{
    return QStringLiteral("noop");
}

NoopSession::Capabilities NoopSession::capabilities() const
{
    return Capabilities();
}

bool NoopSession::isActive() const
{
    return true;
}

QString NoopSession::seat() const
{
    return QStringLiteral("seat0");
}

uint NoopSession::terminal() const
{
    return 0;
}

int NoopSession::openRestricted(const QString &fileName)
{
    return -1;
}

void NoopSession::closeRestricted(int fileDescriptor)
{
}

void NoopSession::switchTo(uint terminal)
{
}

NoopSession *NoopSession::create(QObject *parent)
{
    return new NoopSession(parent);
}

} // namespace Platform

} // namespace Aurora
