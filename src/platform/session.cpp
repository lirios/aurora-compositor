// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "session.h"
#include "session_noop_p.h"
#include "session_logind_p.h"

namespace Aurora {

namespace Platform {

/*!
    \class Session
    \inmodule AuroraCore
    \brief The Session class represents the session controlled by the compositor.

    The Session class provides information about the virtual terminal where the compositor
    is running and a way to open files that require special privileges, e.g. DRM devices or
    input devices.
 */

/*!
    \enum Session::Type

    This enum type is used to specify the type of the session.
 */

/*!
    \enum Session::Capability

    This enum type is used to specify optional capabilities of the session.
 */

/*!
    \fn Capabilities Session::capabilities()

    Returns the capabilities supported by the session.
 */

/*!
    \fn bool Session::isActive()

    Returns \c true if the session is active; otherwise returns \c false.
 */

/*!
    \fn QString Session::seat()

    Returns the seat name for the Session.
 */

/*!
    \fn uint Session::terminal()

    Returns the terminal controlled by the Session.
 */

/*!
    \fn int Session::openRestricted(const QString &fileName)

    Opens the file with the specified \a fileName. Returns the file descriptor
    of the file or \c -1 if an error has occurred.
 */

/*!
    \fn void Session::closeRestricted(int fileDescriptor)

    Closes a file that has been opened using the openRestricted() function.
 */

/*!
    \fn void switchTo(uint terminal)

    Switches to the specified virtual \a terminal. This function does nothing if the
    Capability::SwitchTerminal capability is unsupported.
 */

/*!
    \fn void Session::awoke()

    This signal is emitted when the session is resuming from suspend.
 */

/*!
    \fn void Session::activeChanged(bool active)

    This signal is emitted when the active state of the session has changed.
 */

/*!
    \fn void Session::deviceResumed(dev_t deviceId)

    This signal is emitted when the specified device can be used again.
 */

/*!
    \fn void Session::devicePaused(dev_t deviceId)

    This signal is emitted when the given device cannot be used by the compositor
    anymore. For example, this normally occurs when switching between VTs.

    Note that when this signal is emitted for a DRM device, master permissions can
    be already revoked.
 */

static const struct
{
    Session::Type type;
    std::function<Session *(QObject *parent)> createFunc;
} s_availableSessions[] = {
    { Session::Type::Logind, &LogindSession::create },
    { Session::Type::Noop, &NoopSession::create },
};

Session::Session(QObject *parent)
    : QObject(parent)
{
}

Session *Session::create(QObject *parent)
{
    for (const auto &sessionInfo : s_availableSessions) {
        auto *session = sessionInfo.createFunc(parent);
        if (session)
            return session;
    }

    return nullptr;
}

Session *Session::create(Type type, QObject *parent)
{
    for (const auto &sessionInfo : s_availableSessions) {
        if (sessionInfo.type == type)
            return sessionInfo.createFunc(parent);
    }

    return nullptr;
}

} // namespace Platform

} // namespace Aurora