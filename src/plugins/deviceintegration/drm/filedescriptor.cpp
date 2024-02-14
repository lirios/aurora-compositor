// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2022 Xaver Hugl <xaver.hugl@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "filedescriptor.h"

#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <utility>

namespace Aurora {

namespace Platform {

FileDescriptor::FileDescriptor(int fd)
    : m_fd(fd)
{
}

FileDescriptor::FileDescriptor(FileDescriptor &&other)
    : m_fd(std::exchange(other.m_fd, -1))
{
}

FileDescriptor &FileDescriptor::operator=(FileDescriptor &&other)
{
    if (m_fd != -1) {
        ::close(m_fd);
    }
    m_fd = std::exchange(other.m_fd, -1);
    return *this;
}

FileDescriptor::~FileDescriptor()
{
    if (m_fd != -1) {
        ::close(m_fd);
    }
}

bool FileDescriptor::isValid() const
{
    return m_fd != -1;
}

int FileDescriptor::get() const
{
    return m_fd;
}

int FileDescriptor::take()
{
    return std::exchange(m_fd, -1);
}

void FileDescriptor::reset()
{
    if (m_fd != -1) {
        ::close(m_fd);
        m_fd = -1;
    }
}

FileDescriptor FileDescriptor::duplicate() const
{
    if (m_fd != -1)
        return FileDescriptor{ fcntl(m_fd, F_DUPFD_CLOEXEC, 0) };
    else
        return {};
}

bool FileDescriptor::isClosed() const
{
    return isClosed(m_fd);
}

bool FileDescriptor::isReadable() const
{
    return isReadable(m_fd);
}

bool FileDescriptor::isClosed(int fd)
{
    pollfd pfd = {
        .fd = fd,
        .events = POLLIN,
        .revents = 0,
    };
    if (poll(&pfd, 1, 0) < 0) {
        return true;
    }
    return pfd.revents & (POLLHUP | POLLERR);
}

bool FileDescriptor::isReadable(int fd)
{
    pollfd pfd = {
        .fd = fd,
        .events = POLLIN,
        .revents = 0,
    };
    return poll(&pfd, 1, 0) && (pfd.revents & (POLLIN | POLLNVAL)) != 0;
}

} // namespace Platform

} // namespace Aurora
