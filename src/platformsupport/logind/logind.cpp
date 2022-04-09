/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QCoreApplication>
#include <QDBusConnectionInterface>
#include <QDBusObjectPath>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusUnixFileDescriptor>

#include "defaultlogind_p_p.h"
#include "logind.h"
#include "logind_p.h"

#include <qplatformdefs.h>

#include <sys/sysmacros.h>

Q_LOGGING_CATEGORY(lcLogind, "liri.logind")

#define LOGIN1_SERVICE QStringLiteral("org.freedesktop.login1")

#define LOGIN1_OBJECT QLatin1String("/org/freedesktop/login1")
#define LOGIN1_MANAGER_INTERFACE QLatin1String("org.freedesktop.login1.Manager")
#define LOGIN1_SEAT_INTERFACE QLatin1String("org.freedesktop.login1.Seat")
#define LOGIN1_SESSION_INTERFACE QLatin1String("org.freedesktop.login1.Session")

#define DBUS_SERVICE QLatin1String("org.freedesktop.DBus")
#define DBUS_PROPERTIES_INTERFACE QLatin1String("org.freedesktop.DBus.Properties")

namespace Liri {

/*
 * DefaultLogind
 */

DefaultLogind::DefaultLogind(QObject *parent)
    : Logind(QDBusConnection::systemBus(), parent)
{
}

Q_GLOBAL_STATIC(DefaultLogind, s_logind)

/*
 * LogindPrivate
 */

LogindPrivate::LogindPrivate(Logind *qq)
    : bus(QDBusConnection::systemBus())
    , q_ptr(qq)
{
}

void LogindPrivate::_q_serviceRegistered()
{
    Q_Q(Logind);

    // Skip if we're already connected
    if (isConnected)
        return;

    // Find the active session otherwise try with XDG_SESSION_ID or the PID,
    // when spawned by systemd --user only the first method is expected to work
    DBusUserSession session;
    if (getUserSession(session)) {
        sessionPath = session.objectPath.path();
        qCInfo(lcLogind, "Using session %s", qPrintable(session.id));
    } else {
        QDBusObjectPath sessionObjectPath;

        if (qEnvironmentVariableIsSet("XDG_SESSION_ID")) {
            if (getSessionById(QString::fromLocal8Bit(qgetenv("XDG_SESSION_ID")), sessionObjectPath))
                sessionPath = sessionObjectPath.path();
        }

        if (sessionObjectPath.path().isEmpty()) {
            if (getSessionByPid(sessionObjectPath))
                sessionPath = sessionObjectPath.path();
        }

        if (!sessionPath.isEmpty()) {
            QString sessionId = getSessionId(sessionPath);
            qCInfo(lcLogind, "Using session %s", qPrintable(sessionId));
        }
    }
    if (sessionPath.isEmpty()) {
        qCWarning(lcLogind) << "Unable to find session!";
        return;
    }
    qCDebug(lcLogind) << "Session path:" << sessionPath;

    // We are connected now
    isConnected = true;

    // Listen for lock and unlock signals
    bus.connect(LOGIN1_SERVICE, sessionPath, LOGIN1_SESSION_INTERFACE,
                QLatin1String("Lock"),
                q, SIGNAL(lockSessionRequested()));
    bus.connect(LOGIN1_SERVICE, sessionPath, LOGIN1_SESSION_INTERFACE,
                QLatin1String("Unlock"),
                q, SIGNAL(unlockSessionRequested()));

    // Listen for properties changed
    bus.connect(LOGIN1_SERVICE, sessionPath, DBUS_PROPERTIES_INTERFACE,
                QLatin1String("PropertiesChanged"),
                q, SLOT(_q_sessionPropertiesChanged()));

    // Listen for prepare signals
    bus.connect(LOGIN1_SERVICE, LOGIN1_OBJECT, LOGIN1_MANAGER_INTERFACE,
                QLatin1String("PrepareForSleep"),
                q, SIGNAL(prepareForSleep(bool)));
    bus.connect(LOGIN1_SERVICE, LOGIN1_OBJECT, LOGIN1_MANAGER_INTERFACE,
                QLatin1String("PrepareForShutdown"),
                q, SIGNAL(prepareForShutdown(bool)));

    // Activate the session in case we are on another vt, the
    // call blocks on purpose because we need to get properties
    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           sessionPath,
                                           LOGIN1_MANAGER_INTERFACE,
                                           QLatin1String("Activate"));
    bus.call(message);

    // Get properties
    _q_sessionPropertiesChanged();

    Q_EMIT q->connectedChanged(isConnected);
}

void LogindPrivate::_q_serviceUnregistered()
{
    Q_Q(Logind);

    // Disconnect prepare signals
    bus.disconnect(LOGIN1_SERVICE, LOGIN1_OBJECT, LOGIN1_MANAGER_INTERFACE,
                   QLatin1String("PrepareForSleep"),
                   q, SIGNAL(prepareForSleep(bool)));
    bus.disconnect(LOGIN1_SERVICE, LOGIN1_OBJECT, LOGIN1_MANAGER_INTERFACE,
                   QLatin1String("PrepareForShutdown"),
                   q, SIGNAL(prepareForShutdown(bool)));

    // Connection lost
    isConnected = false;
    Q_EMIT q->connectedChanged(isConnected);

    // Reset properties
    if (sessionActive) {
        sessionActive = false;
        Q_EMIT q->sessionActiveChanged(false);
    }
    if (vt != -1) {
        vt = -1;
        Q_EMIT q->vtNumberChanged(-1);
    }
    if (!seat.isEmpty()) {
        seat = QString();
        Q_EMIT q->seatChanged(seat);
    }
}

void LogindPrivate::_q_sessionPropertiesChanged()
{
    if (!isConnected || sessionPath.isEmpty())
        return;

    getSessionActive();
    getVirtualTerminal();
    getSeat();
}

void LogindPrivate::checkServiceRegistration()
{
    Q_Q(Logind);

    // Get the current session if the logind service is register
    QDBusMessage message =
            QDBusMessage::createMethodCall(DBUS_SERVICE,
                                           QLatin1String("/"),
                                           DBUS_SERVICE,
                                           QLatin1String("ListNames"));

    QDBusPendingReply<QStringList> result = bus.asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(result, q);
    q->connect(watcher, &QDBusPendingCallWatcher::finished, q,
               [this](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<QStringList> reply = *w;
        w->deleteLater();

        if (!reply.isValid())
            return;

        if (reply.value().contains(LOGIN1_SERVICE))
            _q_serviceRegistered();
    });
}

bool LogindPrivate::getSessionById(const QString &sessionId, QDBusObjectPath &path) const
{
    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           LOGIN1_OBJECT,
                                           LOGIN1_MANAGER_INTERFACE,
                                           QStringLiteral("GetSession"));
    message.setArguments(QVariantList() << sessionId);

    QDBusReply<QDBusObjectPath> reply = bus.call(message);
    if (!reply.isValid()) {
        qCWarning(lcLogind, "Failed to get session path for session %s: %s",
                  qPrintable(sessionId),
                  qPrintable(reply.error().message()));
        return false;
    }

    path = reply.value();
    return true;
}

bool LogindPrivate::getSessionByPid(QDBusObjectPath &path) const
{
    QVariantList args;
    args << static_cast<quint32>(QCoreApplication::applicationPid());

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           LOGIN1_OBJECT,
                                           LOGIN1_MANAGER_INTERFACE,
                                           QStringLiteral("GetSessionByPID"));
    message.setArguments(args);

    QDBusReply<QDBusObjectPath> reply = bus.call(message);
    if (!reply.isValid()) {
        qCWarning(lcLogind, "Failed to get session path by PID: %s",
                  qPrintable(reply.error().message()));
        return false;
    }

    path = reply.value();
    return true;
}

bool LogindPrivate::getUserSession(DBusUserSession &session) const
{
    QDBusObjectPath userPath;

    {
        QVariantList args;
        args << ::getuid();

        QDBusMessage message =
                QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                               LOGIN1_OBJECT,
                                               LOGIN1_MANAGER_INTERFACE,
                                               QStringLiteral("GetUser"));
        message.setArguments(args);

        QDBusReply<QDBusObjectPath> reply = bus.call(message);
        if (!reply.isValid()) {
            qCWarning(lcLogind, "Failed to get user path: %s",
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
            qCWarning(lcLogind, "Failed to list user sessions: %s",
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
        bool found = false;
        DBusUserSessionVector sessions = qdbus_cast<DBusUserSessionVector>(reply.value().value<QDBusArgument>());
        for (const auto &curSession : qAsConst(sessions)) {
            const QString type = getSessionType(curSession.id, curSession.objectPath.path());
            const QString state = getSessionState(curSession.id, curSession.objectPath.path());

            if (!validTypes.contains(type))
                continue;
            if (state != QStringLiteral("active"))
                continue;

            // We get the sessions from newest to oldest, pick the last
            // one that meets the criteria
            session = curSession;
            found = true;
        }

        return found;
    }

    return false;
}

QString LogindPrivate::getSessionId(const QString &sessionPath) const
{
    QVariantList args;
    args << LOGIN1_SESSION_INTERFACE
         << QStringLiteral("Id");

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           sessionPath,
                                           DBUS_PROPERTIES_INTERFACE,
                                           QStringLiteral("Get"));
    message.setArguments(args);

    QDBusReply<QVariant> reply = bus.call(message);
    if (!reply.isValid()) {
        qCWarning(lcLogind, "Failed to get session id: %s",
                  qPrintable(reply.error().message()));
        return QString();
    }

    return reply.value().toString();
}

QString LogindPrivate::getSessionType(const QString &sessionId, const QString &sessionPath) const
{
    QVariantList args;
    args << LOGIN1_SESSION_INTERFACE
         << QStringLiteral("Type");

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           sessionPath,
                                           DBUS_PROPERTIES_INTERFACE,
                                           QStringLiteral("Get"));
    message.setArguments(args);

    QDBusReply<QVariant> reply = bus.call(message);
    if (!reply.isValid()) {
        qCWarning(lcLogind, "Failed to get type for session %s: %s",
                  qPrintable(sessionId), qPrintable(reply.error().message()));
        return QString();
    }

    return reply.value().toString();
}

QString LogindPrivate::getSessionState(const QString &sessionId, const QString &sessionPath) const
{
    QVariantList args;
    args << LOGIN1_SESSION_INTERFACE
         << QStringLiteral("State");

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           sessionPath,
                                           DBUS_PROPERTIES_INTERFACE,
                                           QStringLiteral("Get"));
    message.setArguments(args);

    QDBusReply<QVariant> reply = bus.call(message);
    if (!reply.isValid()) {
        qCWarning(lcLogind, "Failed to get state for session %s: %s",
                  qPrintable(sessionId), qPrintable(reply.error().message()));
        return QString();
    }

    return reply.value().toString();
}

void LogindPrivate::getSessionActive()
{
    Q_Q(Logind);

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           sessionPath,
                                           DBUS_PROPERTIES_INTERFACE,
                                           QLatin1String("Get"));
    message.setArguments(QVariantList() << LOGIN1_SESSION_INTERFACE << QStringLiteral("Active"));

    QDBusPendingReply<QVariant> result = bus.asyncCall(message);
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(result, q);
    q->connect(callWatcher, &QDBusPendingCallWatcher::finished, q,
               [q, this](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<QVariant> reply = *w;
        w->deleteLater();

        if (!reply.isValid()) {
            qCWarning(lcLogind, "Failed to get \"Active\" property from session: %s",
                      qPrintable(reply.error().message()));
            return;
        }

        const bool active = reply.value().toBool();
        if (sessionActive != active) {
            sessionActive = active;
            Q_EMIT q->sessionActiveChanged(active);
        }
    });
}

void LogindPrivate::getVirtualTerminal()
{
    Q_Q(Logind);

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           sessionPath,
                                           DBUS_PROPERTIES_INTERFACE,
                                           QLatin1String("Get"));
    message.setArguments(QVariantList() << LOGIN1_SESSION_INTERFACE << QStringLiteral("VTNr"));

    QDBusPendingReply<QVariant> result = bus.asyncCall(message);
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(result, q);
    q->connect(callWatcher, &QDBusPendingCallWatcher::finished, q,
               [q, this](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<QVariant> reply = *w;
        w->deleteLater();

        if (!reply.isValid()) {
            qCWarning(lcLogind, "Failed to get \"VTNr\" property from session: %s",
                      qPrintable(reply.error().message()));
            return;
        }

        const uint vtnr = reply.value().toUInt();
        if (vt != static_cast<int>(vtnr)) {
            vt = static_cast<int>(vtnr);
            Q_EMIT q->vtNumberChanged(vt);
        }
    });
}

void LogindPrivate::getSeat()
{
    Q_Q(Logind);

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           sessionPath,
                                           DBUS_PROPERTIES_INTERFACE,
                                           QLatin1String("Get"));
    message.setArguments(QVariantList() << LOGIN1_SESSION_INTERFACE << QStringLiteral("Seat"));

    QDBusPendingReply<QVariant> result = bus.asyncCall(message);
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(result, q);
    q->connect(callWatcher, &QDBusPendingCallWatcher::finished, q,
               [q, this](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<QVariant> reply = *w;
        w->deleteLater();

        if (!reply.isValid()) {
            qCWarning(lcLogind, "Failed to get \"Seat\" property from session: %s",
                      qPrintable(reply.error().message()));
            return;
        }

        const DBusSeat dbusSeat = qdbus_cast<DBusSeat>(reply.value().value<QDBusArgument>());
        if (seat != dbusSeat.id) {
            seat = dbusSeat.id;
            Q_EMIT q->seatChanged(seat);
        }
    });
}

/*
 * Logind
 */

/*!
 * \class Logind
 * \inmodule LiriLogind
 * \brief Qt-style API for logind.
 *
 * LiriLogind is a Qt-style API for logind.
 *
 * More information on logind can be obtained
 * \l{https://www.freedesktop.org/wiki/Software/systemd/logind/}(here).
 *
 * Logging category is "liri.logind".
 *
 * This class is a singleton, you cannot instantiate it. Use the instance()
 * method to get access.
 */

/*!
 * Constructs a Logind instance with the given \a parent and D-Bus \a connection.
 */
Logind::Logind(const QDBusConnection &connection, QObject *parent)
    : QObject(parent)
    , d_ptr(new LogindPrivate(this))
{
    // Register custom types
    qDBusRegisterMetaType<DBusUserSession>();
    qDBusRegisterMetaType<DBusUserSessionVector>();
    qDBusRegisterMetaType<DBusSeat>();

    // Initialize and respond to logind service (un)registration
    Q_D(Logind);
    d->bus = connection;
    d->watcher =
            new QDBusServiceWatcher(LOGIN1_SERVICE, d->bus,
                                    QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
                                    this);
    connect(d->watcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(_q_serviceRegistered()));
    connect(d->watcher, SIGNAL(serviceUnregistered(QString)),
            this, SLOT(_q_serviceUnregistered()));

    // Is logind already registered?
    d->checkServiceRegistration();
}

/*!
 * Destroys the Logind object.
 */
Logind::~Logind()
{
    delete d_ptr;
}

/*!
 * Return the instance of Logind.
 */
Logind *Logind::instance()
{
    return s_logind();
}

bool Logind::checkService()
{
    QDBusConnectionInterface *interface = QDBusConnection::systemBus().interface();
    return interface->isServiceRegistered(LOGIN1_SERVICE);
}

/*!
 * \property Logind::isConnected
 *
 * This property holds whether we are connected to logind.
 */
bool Logind::isConnected() const
{
    Q_D(const Logind);
    return d->isConnected;
}

/*!
 * \property Logind::isConnected
 *
 * This property holds whether session control was acquired.
 *
 * \sa Logind::takeControl()
 * \sa Logind::releaseControl()
 */
bool Logind::hasSessionControl() const
{
    Q_D(const Logind);
    return d->hasSessionControl;
}

/*!
 * \property Logind::isSessionActive
 *
 * This property holds whether the session this process lives in
 * is active or not, that is whether the current vt is the one
 * where the session was executed.
 */
bool Logind::isSessionActive() const
{
    Q_D(const Logind);
    return d->sessionActive;
}

/*!
 * \property Logind::isInhibited
 *
 * This property holds if there are inhibitions installed.
 */
bool Logind::isInhibited() const
{
    Q_D(const Logind);
    return d->inhibitFds.size() > 0;
}

/*!
 * \property Logind::vtNumber
 *
 * This property holds the current vt number.
 */
int Logind::vtNumber() const
{
    Q_D(const Logind);
    return d->vt;
}

/*!
 * \property Logind::seat
 *
 * This property holds the current seat.
 */
QString Logind::seat() const
{
    Q_D(const Logind);
    return d->seat;
}

/*!
 * Set the idle hint state to \a idle.
 */
void Logind::setIdleHint(bool idle)
{
    Q_D(Logind);

    if (!d->isConnected || d->sessionPath.isEmpty())
        return;

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           d->sessionPath,
                                           LOGIN1_SESSION_INTERFACE,
                                           QStringLiteral("SetIdleHint"));
    d->bus.asyncCall(message, idle);
}

/*!
 * Install an inhibitor.
 * \param who Name of the application that is installing the inhibitor
 * \param why Reason why the inhibitor is being installed
 * \param flags What is being inhibited
 * \param mode Inhibition mode
 *
 * \sa Logind::inhibited()
 * \sa Logind::uninhibited()
 */
void Logind::inhibit(const QString &who, const QString &why,
                     InhibitFlags flags, InhibitMode mode)
{
    Q_D(Logind);

    if (!d->isConnected)
        return;

    QStringList what;
    if (flags.testFlag(InhibitShutdown))
        what.append(QStringLiteral("shutdown"));
    if (flags.testFlag(InhibitSleep))
        what.append(QStringLiteral("sleep"));
    if (flags.testFlag(InhibitIdle))
        what.append(QStringLiteral("idle"));
    if (flags.testFlag(InhibitPowerKey))
        what.append(QStringLiteral("handle-power-key"));
    if (flags.testFlag(InhibitSuspendKey))
        what.append(QStringLiteral("handle-suspend-key"));
    if (flags.testFlag(InhibitHibernateKey))
        what.append(QStringLiteral("handle-hibernate-key"));
    if (flags.testFlag(InhibitLidSwitch))
        what.append(QStringLiteral("handle-lid-switch"));

    QString modeStr = mode == Block ? QStringLiteral("block") : QStringLiteral("delay");

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           LOGIN1_OBJECT,
                                           LOGIN1_MANAGER_INTERFACE,
                                           QLatin1String("Inhibit"));
    message.setArguments(QVariantList() << what.join(':') << who << why << modeStr);

    QDBusPendingReply<QDBusUnixFileDescriptor> result = d->bus.asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(result, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [d, this, who, why](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<QDBusUnixFileDescriptor> reply = *w;
        w->deleteLater();

        if (!reply.isValid()) {
            qCWarning(lcLogind, "Unable to acquire inhibition lock: %s",
                      qPrintable(reply.error().message()));
            return;
        }

        qCDebug(lcLogind) << "Inhibition lock acquired successfully";

        const int fd = ::dup(reply.value().fileDescriptor());
        d->inhibitFds.append(fd);
        if (d->inhibitFds.size() == 1)
            Q_EMIT inhibitedChanged(true);
        Q_EMIT inhibited(who, why, fd);
    });
}

/*!
 * Uninstall the inhibitor with \a fd file descriptor.
 */
void Logind::uninhibit(int fd)
{
    Q_D(Logind);

    if (!d->isConnected || !d->inhibitFds.contains(fd))
        return;

    ::close(fd);
    d->inhibitFds.removeOne(fd);

    if (d->inhibitFds.size() == 0)
        Q_EMIT inhibitedChanged(false);
    Q_EMIT uninhibited(fd);
}

/*!
 * Lock the session.
 *
 * \sa Logind::unlockSession()
 */
void Logind::lockSession()
{
    Q_D(Logind);

    if (!d->isConnected || d->sessionPath.isEmpty())
        return;

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           d->sessionPath,
                                           LOGIN1_SESSION_INTERFACE,
                                           QLatin1String("Lock"));
    d->bus.asyncCall(message);
}

/*!
 * Unlock the session.
 *
 * \sa Logind::lockSession()
 */
void Logind::unlockSession()
{
    Q_D(Logind);

    if (!d->isConnected || d->sessionPath.isEmpty())
        return;

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           d->sessionPath,
                                           LOGIN1_SESSION_INTERFACE,
                                           QLatin1String("Unlock"));
    d->bus.asyncCall(message);
}

/*!
 * Take control of the session. No other process will be
 * able to take control, until the releaaseControl() method is called.
 *
 * Access to video and input devices will be granted to the caller.
 *
 * \sa Logind::releaseControl()
 */
void Logind::takeControl()
{
    Q_D(Logind);

    if (!d->isConnected || d->sessionPath.isEmpty() || d->hasSessionControl)
        return;

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           d->sessionPath,
                                           LOGIN1_SESSION_INTERFACE,
                                           QLatin1String("TakeControl"));
    message.setArguments(QVariantList() << false);

    QDBusPendingReply<void> result = d->bus.asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(result, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [d, this](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<void> reply = *w;
        w->deleteLater();

        if (!reply.isValid()) {
            qCWarning(lcLogind, "Unable to take control of the session: %s",
                      qPrintable(reply.error().message()));
            d->hasSessionControl = false;
            Q_EMIT hasSessionControlChanged(d->hasSessionControl);
            return;
        }

        qCDebug(lcLogind) << "Acquired control of the session";
        d->hasSessionControl = true;
        Q_EMIT hasSessionControlChanged(d->hasSessionControl);

        d->bus.connect(LOGIN1_SERVICE, d->sessionPath, LOGIN1_SESSION_INTERFACE,
                       QLatin1String("PauseDevice"),
                       this, SIGNAL(devicePaused(quint32,quint32,QString)));
        d->bus.connect(LOGIN1_SERVICE, d->sessionPath, LOGIN1_SESSION_INTERFACE,
                       QLatin1String("ResumeDevice"),
                       this, SIGNAL(deviceResumed(quint32,quint32,int)));
    });
}

/*!
 * Release control of the session.
 *
 * Access to video and input devices will be revoked from the caller.
 *
 * \sa Logind::takeControl()
 */
void Logind::releaseControl()
{
    Q_D(Logind);

    if (!d->isConnected || d->sessionPath.isEmpty() || !d->hasSessionControl)
        return;

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           d->sessionPath,
                                           LOGIN1_SESSION_INTERFACE,
                                           QLatin1String("ReleaseControl"));
    d->bus.asyncCall(message);

    qCDebug(lcLogind) << "Released control of the session";
    d->hasSessionControl = false;
    Q_EMIT hasSessionControlChanged(d->hasSessionControl);
}

/*!
 * Request access to the device \a fileName.
 *
 * \return File descriptor of the device
 *
 * \sa Logind::releaseDevice()
 */
int Logind::takeDevice(const QString &fileName)
{
    Q_D(Logind);

    struct stat st;
    if (::stat(qPrintable(fileName), &st) < 0) {
        qCWarning(lcLogind, "Failed to stat: %s", qPrintable(fileName));
        return -1;
    }

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           d->sessionPath,
                                           LOGIN1_SESSION_INTERFACE,
                                           QLatin1String("TakeDevice"));
    message.setArguments(QVariantList()
                         << QVariant(major(st.st_rdev))
                         << QVariant(minor(st.st_rdev)));

    // Block until the device is taken
    QDBusMessage reply = d->bus.call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCWarning(lcLogind, "Failed to take device \"%s\": %s",
                  qPrintable(fileName), qPrintable(reply.errorMessage()));
        return -1;
    }

    const int fd = reply.arguments().at(0).value<QDBusUnixFileDescriptor>().fileDescriptor();
    return ::fcntl(fd, F_DUPFD_CLOEXEC, 0);
}

/*
 * Revoke access to the device \a fd.
 *
 * \sa Logind::takeDevice()
 */
void Logind::releaseDevice(int fd)
{
    Q_D(Logind);

    struct stat st;
    if (::fstat(fd, &st) < 0) {
        qCWarning(lcLogind, "Failed to stat the file descriptor");
        return;
    }

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           d->sessionPath,
                                           LOGIN1_SESSION_INTERFACE,
                                           QLatin1String("ReleaseDevice"));
    message.setArguments(QVariantList()
                         << QVariant(major(st.st_rdev))
                         << QVariant(minor(st.st_rdev)));

    d->bus.asyncCall(message);
}

/*!
 * Allow a session-controller to synchronously pause a device
 * with \a devMajor major and \a devMinor minor after receiving
 * the devicePaused signal.
 *
 * \sa Logind::devicePaused()
 */
void Logind::pauseDeviceComplete(quint32 devMajor, quint32 devMinor)
{
    Q_D(Logind);

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           d->sessionPath,
                                           LOGIN1_SESSION_INTERFACE,
                                           QLatin1String("PauseDeviceComplete"));
    message.setArguments(QVariantList() << devMajor << devMinor);

    d->bus.asyncCall(message);
}

/*!
 * Switch to the vt number \a vt.
 */
void Logind::switchTo(quint32 vt)
{
    Q_D(Logind);

    QDBusMessage message =
            QDBusMessage::createMethodCall(LOGIN1_SERVICE,
                                           QLatin1String("/org/freedesktop/login1/seat/self"),
                                           LOGIN1_SEAT_INTERFACE,
                                           QLatin1String("SwitchTo"));
    message.setArguments(QVariantList() << QVariant(vt));

    d->bus.asyncCall(message);
}

} // namespace Liri

#include "moc_logind.cpp"
