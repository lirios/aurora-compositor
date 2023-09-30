// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2022 Xaver Hugl <xaver.hugl@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

namespace Aurora {

namespace Platform {

class FileDescriptor
{
public:
    FileDescriptor() = default;
    explicit FileDescriptor(int fd);
    FileDescriptor(FileDescriptor &&);
    FileDescriptor &operator=(FileDescriptor &&);
    ~FileDescriptor();

    bool isValid() const;
    int get() const;
    int take();
    void reset();
    FileDescriptor duplicate() const;

    bool isReadable() const;
    bool isClosed() const;

    static bool isReadable(int fd);
    static bool isClosed(int fd);

private:
    int m_fd = -1;
};

} // namespace Platform

} // namespace Aurora
