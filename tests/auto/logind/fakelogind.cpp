/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:GPL3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QDBusConnection>

#include "fakelogind.h"

/*
 * FakeLogindSession
 */

FakeLogindSession::FakeLogindSession(const QString &path, QObject *parent)
    : QObject(parent)
    , m_path(path)
{
    QDBusConnection::sessionBus().registerObject(
                m_path, this, QDBusConnection::ExportScriptableContents);
}

FakeLogindSession::~FakeLogindSession()
{
    QDBusConnection::sessionBus().unregisterObject(m_path);
}

const QString &FakeLogindSession::path()
{
    return m_path;
}

bool FakeLogindSession::isActive() const
{
    return true;
}

quint32 FakeLogindSession::vtNumber() const
{
    return 1;
}

void FakeLogindSession::TakeControl(bool force)
{
    Q_UNUSED(force);
}

void FakeLogindSession::ReleaseControl()
{
}

/*
 * FakeLogind
 */

FakeLogind::FakeLogind(QObject *parent)
    : QObject(parent)
    , m_session(new FakeLogindSession(QStringLiteral("/org/freedesktop/login1/session/_1"), this))
{
    QDBusConnection::sessionBus().registerObject(
                QStringLiteral("/org/freedesktop/login1"), this,
                QDBusConnection::ExportScriptableContents);
    QDBusConnection::sessionBus().registerService(
                QStringLiteral("org.freedesktop.login1"));
}

FakeLogind::~FakeLogind()
{
    QDBusConnection::sessionBus().unregisterObject(
                QStringLiteral("/org/freedesktop/login1"));
    QDBusConnection::sessionBus().unregisterService(
                QStringLiteral("org.freedesktop.login1"));
}

void FakeLogind::doLock()
{
    Q_EMIT m_session->Lock();
}

void FakeLogind::doUnlock()
{
    Q_EMIT m_session->Unlock();
}

void FakeLogind::doPrepareForSleep(bool before)
{
    Q_EMIT PrepareForSleep(before);
}

void FakeLogind::doPrepareForShutdown(bool before)
{
    Q_EMIT PrepareForShutdown(before);
}

QDBusObjectPath FakeLogind::GetSessionByPID(quint32 pid)
{
    Q_UNUSED(pid);
    return QDBusObjectPath(m_session->path());
}

int FakeLogind::TakeDevice(int maj, int min)
{
    Q_UNUSED(maj);
    Q_UNUSED(min);
    return -1;
}

void FakeLogind::ReleaseDevice(int maj, int min)
{
    Q_UNUSED(maj);
    Q_UNUSED(min);
}
