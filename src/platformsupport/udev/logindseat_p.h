/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPLv3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#ifndef LOGINDSEAT_P_H
#define LOGINDSEAT_P_H

#include <QDBusArgument>
#include <QVector>

class DBusUserSession
{
public:
    QString id;
    QDBusObjectPath objectPath;
};
Q_DECLARE_METATYPE(DBusUserSession)

typedef QVector<DBusUserSession> DBusUserSessionVector;
Q_DECLARE_METATYPE(DBusUserSessionVector)

class DBusSeat
{
public:
    QString id;
    QDBusObjectPath objectPath;
};
Q_DECLARE_METATYPE(DBusSeat)

QDBusArgument &operator<<(QDBusArgument &argument, const DBusUserSession &userSession);
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusUserSession &userSession);

QDBusArgument &operator<<(QDBusArgument &argument, const DBusSeat &seat);
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusSeat &seat);

class LogindSeat
{
public:
    LogindSeat();

    QString id() const;

private:
    QString m_id = QStringLiteral("seat0");

    bool findActiveSession(DBusUserSession &userSession) const;
    QString getSessionType(const QString &sessionId, const QString &sessionPath) const;
    QString getSessionState(const QString &sessionId, const QString &sessionPath) const;
    QString getSeatId(const QString &sessionPath) const;
};

#endif // LOGINDSEAT_P_H
