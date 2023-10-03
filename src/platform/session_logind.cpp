// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusPendingCall>

#include "auroraplatformloggingcategories.h"
#include "session_logind_p.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#if __has_include(<sys/sysmacros.h>)
#  include <sys/sysmacros.h>
#endif

struct DBusLogindSeat
{
    QString id;
    QDBusObjectPath path;
};

QDBusArgument &operator<<(QDBusArgument &argument, const DBusLogindSeat &seat)
{
    argument.beginStructure();
    argument << seat.id << seat.path;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusLogindSeat &seat)
{
    argument.beginStructure();
    argument >> seat.id >> seat.path;
    argument.endStructure();
    return argument;
}

Q_DECLARE_METATYPE(DBusLogindSeat)

namespace Aurora {

namespace Platform {

static const QString s_serviceName = QStringLiteral("org.freedesktop.login1");
static const QString s_propertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
static const QString s_sessionInterface = QStringLiteral("org.freedesktop.login1.Session");
static const QString s_seatInterface = QStringLiteral("org.freedesktop.login1.Seat");
static const QString s_managerInterface = QStringLiteral("org.freedesktop.login1.Manager");
static const QString s_managerPath = QStringLiteral("/org/freedesktop/login1");

static QString findProcessSessionPath()
{
    const QString sessionId = qEnvironmentVariable("XDG_SESSION_ID", QStringLiteral("auto"));

    QDBusMessage message = QDBusMessage::createMethodCall(
            s_serviceName, s_managerPath, s_managerInterface, QStringLiteral("GetSession"));
    message.setArguments({ sessionId });

    const QDBusMessage reply = QDBusConnection::systemBus().call(message);
    if (reply.type() == QDBusMessage::ErrorMessage)
        return QString();

    return reply.arguments().constFirst().value<QDBusObjectPath>().path();
}

static bool takeControl(const QString &sessionPath)
{
    QDBusMessage message = QDBusMessage::createMethodCall(
            s_serviceName, sessionPath, s_sessionInterface, QStringLiteral("TakeControl"));
    message.setArguments({ false });

    const QDBusMessage reply = QDBusConnection::systemBus().call(message);

    return reply.type() != QDBusMessage::ErrorMessage;
}

static void releaseControl(const QString &sessionPath)
{
    const QDBusMessage message = QDBusMessage::createMethodCall(
            s_serviceName, sessionPath, s_sessionInterface, QStringLiteral("ReleaseControl"));

    QDBusConnection::systemBus().asyncCall(message);
}

static bool activate(const QString &sessionPath)
{
    const QDBusMessage message = QDBusMessage::createMethodCall(
            s_serviceName, sessionPath, s_sessionInterface, QStringLiteral("Activate"));

    const QDBusMessage reply = QDBusConnection::systemBus().call(message);

    return reply.type() != QDBusMessage::ErrorMessage;
}

LogindSession::LogindSession(const QString &sessionPath, QObject *parent)
    : Session(parent)
    , m_sessionPath(sessionPath)
{
    qDBusRegisterMetaType<DBusLogindSeat>();
}

bool LogindSession::initialize()
{
    QDBusMessage activeMessage = QDBusMessage::createMethodCall(
            s_serviceName, m_sessionPath, s_propertiesInterface, QStringLiteral("Get"));
    activeMessage.setArguments({ s_sessionInterface, QStringLiteral("Active") });

    QDBusMessage seatMessage = QDBusMessage::createMethodCall(
            s_serviceName, m_sessionPath, s_propertiesInterface, QStringLiteral("Get"));
    seatMessage.setArguments({ s_sessionInterface, QStringLiteral("Seat") });

    QDBusMessage terminalMessage = QDBusMessage::createMethodCall(
            s_serviceName, m_sessionPath, s_propertiesInterface, QStringLiteral("Get"));
    terminalMessage.setArguments({ s_sessionInterface, QStringLiteral("VTNr") });

    QDBusPendingReply<QVariant> activeReply = QDBusConnection::systemBus().asyncCall(activeMessage);
    QDBusPendingReply<QVariant> terminalReply =
            QDBusConnection::systemBus().asyncCall(terminalMessage);
    QDBusPendingReply<QVariant> seatReply = QDBusConnection::systemBus().asyncCall(seatMessage);

    // We must wait until all replies have been received because the drm backend needs a
    // valid seat name to properly select gpu devices, this also simplifies startup code.
    activeReply.waitForFinished();
    terminalReply.waitForFinished();
    seatReply.waitForFinished();

    if (activeReply.isError()) {
        qCWarning(gLcAuroraPlatform)
                << "Failed to query Active session property:" << activeReply.error();
        return false;
    }
    if (terminalReply.isError()) {
        qCWarning(gLcAuroraPlatform)
                << "Failed to query VTNr session property:" << terminalReply.error();
        return false;
    }
    if (seatReply.isError()) {
        qCWarning(gLcAuroraPlatform)
                << "Failed to query Seat session property:" << seatReply.error();
        return false;
    }

    m_isActive = activeReply.value().toBool();
    m_terminal = terminalReply.value().toUInt();

    const DBusLogindSeat seat =
            qdbus_cast<DBusLogindSeat>(seatReply.value().value<QDBusArgument>());
    m_seatId = seat.id;
    m_seatPath = seat.path.path();

    QDBusConnection::systemBus().connect(s_serviceName, s_managerPath, s_managerInterface,
                                         QStringLiteral("PrepareForSleep"), this,
                                         SLOT(handlePrepareForSleep(bool)));

    QDBusConnection::systemBus().connect(s_serviceName, m_sessionPath, s_sessionInterface,
                                         QStringLiteral("PauseDevice"), this,
                                         SLOT(handlePauseDevice(uint, uint, QString)));

    QDBusConnection::systemBus().connect(
            s_serviceName, m_sessionPath, s_sessionInterface, QStringLiteral("ResumeDevice"), this,
            SLOT(handleResumeDevice(uint, uint, QDBusUnixFileDescriptor)));

    QDBusConnection::systemBus().connect(s_serviceName, m_sessionPath, s_propertiesInterface,
                                         QStringLiteral("PropertiesChanged"), this,
                                         SLOT(handlePropertiesChanged(QString, QVariantMap)));

    return true;
}

void LogindSession::updateActive(bool active)
{
    if (m_isActive != active) {
        m_isActive = active;
        emit activeChanged(active);
    }
}

LogindSession::~LogindSession()
{
    releaseControl(m_sessionPath);
}

QString LogindSession::name() const
{
    return QStringLiteral("logind");
}

LogindSession::Capabilities LogindSession::capabilities() const
{
    return Capability::SwitchTerminal;
}

bool LogindSession::isActive() const
{
    return m_isActive;
}

QString LogindSession::seat() const
{
    return m_seatId;
}

uint LogindSession::terminal() const
{
    return m_terminal;
}

int LogindSession::openRestricted(const QString &fileName)
{
    struct stat st;
    if (stat(qPrintable(fileName), &st) < 0)
        return -1;

    QDBusMessage message = QDBusMessage::createMethodCall(
            s_serviceName, m_sessionPath, s_sessionInterface, QStringLiteral("TakeDevice"));
    // major() and minor() macros return ints on FreeBSD instead of uints.
    message.setArguments({ uint(major(st.st_rdev)), uint(minor(st.st_rdev)) });

    const QDBusMessage reply = QDBusConnection::systemBus().call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qCWarning(gLcAuroraPlatform, "Failed to open %s device (%s)", qPrintable(fileName),
                  qPrintable(reply.errorMessage()));
        return -1;
    }

    const QDBusUnixFileDescriptor descriptor =
            reply.arguments().constFirst().value<QDBusUnixFileDescriptor>();
    if (!descriptor.isValid()) {
        qCWarning(gLcAuroraPlatform, "File descriptor for %s from logind is invalid",
                  qPrintable(fileName));
        return -1;
    }

    return fcntl(descriptor.fileDescriptor(), F_DUPFD_CLOEXEC, 0);
}

void LogindSession::closeRestricted(int fileDescriptor)
{
    struct stat st;
    if (fstat(fileDescriptor, &st) < 0) {
        close(fileDescriptor);
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(
            s_serviceName, m_sessionPath, s_sessionInterface, QStringLiteral("ReleaseDevice"));
    // major() and minor() macros return ints on FreeBSD instead of uints.
    message.setArguments({ uint(major(st.st_rdev)), uint(minor(st.st_rdev)) });

    QDBusConnection::systemBus().asyncCall(message);

    close(fileDescriptor);
}

void LogindSession::switchTo(uint terminal)
{
    QDBusMessage message = QDBusMessage::createMethodCall(
            s_serviceName, m_seatPath, s_seatInterface, QStringLiteral("SwitchTo"));
    message.setArguments({ terminal });

    QDBusConnection::systemBus().asyncCall(message);
}

LogindSession *LogindSession::create(QObject *parent)
{
    QDBusConnectionInterface *interface = QDBusConnection::systemBus().interface();
    if (!interface->isServiceRegistered(s_serviceName)) {
        qCWarning(gLcAuroraPlatform) << "Service" << s_serviceName << "not registered";
        return nullptr;
    }

    const QString sessionPath = findProcessSessionPath();
    if (sessionPath.isEmpty()) {
        qCWarning(gLcAuroraPlatform) << "Could not determine the active graphical session";
        return nullptr;
    }

    if (!activate(sessionPath)) {
        qCWarning(gLcAuroraPlatform,
                  "Failed to activate \"%s\" session. Maybe another compositor is running?",
                  qPrintable(sessionPath));
        return nullptr;
    }

    if (!takeControl(sessionPath)) {
        qCWarning(gLcAuroraPlatform,
                  "Failed to take control of %s session. Maybe another compositor is running?",
                  qPrintable(sessionPath));
        return nullptr;
    }

    auto *session = new LogindSession(sessionPath, parent);
    if (session->initialize())
        return session;
    else
        session->deleteLater();

    return nullptr;
}

void LogindSession::handlePauseDevice(uint major, uint minor, const QString &type)
{
    emit devicePaused(makedev(major, minor));

    if (type == QLatin1String("pause")) {
        QDBusMessage message =
                QDBusMessage::createMethodCall(s_serviceName, m_sessionPath, s_sessionInterface,
                                               QStringLiteral("PauseDeviceComplete"));
        message.setArguments({ major, minor });

        QDBusConnection::systemBus().asyncCall(message);
    }
}

void LogindSession::handleResumeDevice(uint major, uint minor,
                                       QDBusUnixFileDescriptor fileDescriptor)
{
    Q_UNUSED(fileDescriptor);

    // We don't care about the file descriptor as the libinput backend will re-open input devices
    // and the drm file descriptors remain valid after pausing gpus.

    emit deviceResumed(makedev(major, minor));
}

void LogindSession::handlePropertiesChanged(const QString &interfaceName,
                                            const QVariantMap &properties)
{
    if (interfaceName == s_sessionInterface) {
        const QVariant active = properties.value(QStringLiteral("Active"));
        if (active.isValid())
            updateActive(active.toBool());
    }
}

void LogindSession::handlePrepareForSleep(bool sleep)
{
    if (!sleep)
        emit awoke();
}

} // namespace Platform

} // namespace Aurora
