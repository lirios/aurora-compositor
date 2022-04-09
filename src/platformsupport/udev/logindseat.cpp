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

#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusReply>

#include "logindseat_p.h"
#include "udev_p.h"

#include <unistd.h>
#include <sys/types.h>

#define LOGIN1_SERVICE QStringLiteral("org.freedesktop.login1")

#define LOGIN1_OBJECT QLatin1String("/org/freedesktop/login1")
#define LOGIN1_MANAGER_INTERFACE QLatin1String("org.freedesktop.login1.Manager")
#define LOGIN1_SEAT_INTERFACE QLatin1String("org.freedesktop.login1.Seat")
#define LOGIN1_SESSION_INTERFACE QLatin1String("org.freedesktop.login1.Session")

#define DBUS_SERVICE QLatin1String("org.freedesktop.DBus")
#define DBUS_PROPERTIES_INTERFACE QLatin1String("org.freedesktop.DBus.Properties")

LogindSeat::LogindSeat()
{
    // Register custom types
    qDBusRegisterMetaType<DBusUserSession>();
    qDBusRegisterMetaType<DBusUserSessionVector>();
    qDBusRegisterMetaType<DBusSeat>();

    DBusUserSession userSession;
    if (findActiveSession(userSession))
        m_id = getSeatId(userSession.objectPath.path());
    else
        qCWarning(lcUdev, "Failed to determine current seat, assuming default seat0");
}

QString LogindSeat::id() const
{
    return m_id;
}

bool LogindSeat::findActiveSession(DBusUserSession &userSession) const
{
    auto bus = QDBusConnection::systemBus();

    QDBusObjectPath userPath;

    {
        QVariantList args;
        args << ::getuid();

        auto message =
                QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                               LOGIN1_OBJECT,
                                               LOGIN1_MANAGER_INTERFACE,
                                               QStringLiteral("GetUser"));
        message.setArguments(args);

        QDBusReply<QDBusObjectPath> reply = bus.call(message);
        if (!reply.isValid()) {
            qCWarning(lcUdev, "Failed to get user path: %s",
                      qPrintable(reply.error().message()));
            return false;
        }

        userPath = reply.value();
    }

    {
        QVariantList args;
        args << QStringLiteral("org.freedesktop.login1.User")
             << QStringLiteral("Sessions");

        QDBusMessage message =
                QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                               userPath.path(),
                                               DBUS_PROPERTIES_INTERFACE,
                                               QStringLiteral("Get"));
        message.setArguments(args);

        QDBusReply<QVariant> reply = bus.call(message);
        if (!reply.isValid()) {
            qCWarning(lcUdev, "Failed to list user sessions: %s",
                      qPrintable(reply.error().message()));
            return false;
        }

        // Find which session meets our critera
        QStringList validTypes = {
            QStringLiteral("tty"),
            QStringLiteral("wayland"),
            QStringLiteral("x11")
        };

        // We expect to have only one session for each user, and the session for the current
        // user is supposed to be active because the user logged in with a login manager (either
        // text based such as getty, or graphical like SDDM).
        // Graphical login managers usually don't spawn a second session, but activate an already
        // existing session for the user.
        DBusUserSessionVector sessions = qdbus_cast<DBusUserSessionVector>(reply.value().value<QDBusArgument>());
        for (const auto &curSession : qAsConst(sessions)) {
            const QString type = getSessionType(curSession.id, curSession.objectPath.path());
            const QString state = getSessionState(curSession.id, curSession.objectPath.path());

            if (!validTypes.contains(type))
                continue;

            // We want the active session
            if (state == QStringLiteral("active")) {
                userSession = curSession;
                return true;
            }
        }
    }

    return false;
}

QString LogindSeat::getSessionType(const QString &sessionId, const QString &sessionPath) const
{
    auto bus = QDBusConnection::systemBus();

    QVariantList args;
    args << LOGIN1_SESSION_INTERFACE
         << QStringLiteral("Type");

    auto message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           sessionPath,
                                           DBUS_PROPERTIES_INTERFACE,
                                           QStringLiteral("Get"));
    message.setArguments(args);

    QDBusReply<QVariant> reply = bus.call(message);
    if (!reply.isValid()) {
        qCWarning(lcUdev, "Failed to get type for session %s: %s",
                  qPrintable(sessionId), qPrintable(reply.error().message()));
        return QString();
    }

    return reply.value().toString();
}

QString LogindSeat::getSessionState(const QString &sessionId, const QString &sessionPath) const
{
    auto bus = QDBusConnection::systemBus();

    QVariantList args;
    args << LOGIN1_SESSION_INTERFACE
         << QStringLiteral("State");

    auto message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           sessionPath,
                                           DBUS_PROPERTIES_INTERFACE,
                                           QStringLiteral("Get"));
    message.setArguments(args);

    QDBusReply<QVariant> reply = bus.call(message);
    if (!reply.isValid()) {
        qCWarning(lcUdev, "Failed to get state for session %s: %s",
                  qPrintable(sessionId), qPrintable(reply.error().message()));
        return QString();
    }

    return reply.value().toString();
}

QString LogindSeat::getSeatId(const QString &sessionPath) const
{
    auto bus = QDBusConnection::systemBus();

    QVariantList args;
    args << LOGIN1_SESSION_INTERFACE
         << QStringLiteral("Seat");

    auto message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           sessionPath,
                                           DBUS_PROPERTIES_INTERFACE,
                                           QStringLiteral("Get"));
    message.setArguments(args);

    QDBusReply<QVariant> reply = bus.call(message);
    if (!reply.isValid()) {
        qCWarning(lcUdev, "Failed to get seat from session path \"%s\": %s",
                  qPrintable(sessionPath), qPrintable(reply.error().message()));
        return QString();
    }

    const DBusSeat dbusSeat = qdbus_cast<DBusSeat>(reply.value().value<QDBusArgument>());
    return dbusSeat.id;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DBusUserSession &userSession)
{
    argument.beginStructure();
    argument << userSession.id << userSession.objectPath;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusUserSession &userSession)
{
    argument.beginStructure();
    argument >> userSession.id >> userSession.objectPath;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DBusSeat &seat)
{
    argument.beginStructure();
    argument << seat.id << seat.objectPath;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusSeat &seat)
{
    argument.beginStructure();
    argument >> seat.id >> seat.objectPath;
    argument.endStructure();
    return argument;
}
