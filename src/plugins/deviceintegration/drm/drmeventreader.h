// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2019 The Qt Company Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPointer>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

namespace Aurora {

namespace Platform {

class DrmDevice;

struct DrmEventHost : public QObject
{
    struct PendingFlipWait
    {
        void *key;
        QMutex *mutex;
        QWaitCondition *cond;
    };

    static const int MAX_FLIPS = 32;
    void *completedFlips[MAX_FLIPS] = {};
    DrmEventHost::PendingFlipWait pendingFlipWaits[MAX_FLIPS] = {};

    bool event(QEvent *event) override;
    void updateStatus();
    void handlePageFlipCompleted(void *key);
};

class DrmEventReaderThread : public QThread
{
    Q_OBJECT
public:
    explicit DrmEventReaderThread(int fd, QObject *parent = nullptr);
    ~DrmEventReaderThread();

    DrmEventHost *eventHost() const;

protected:
    void run() override;

private:
    int m_fd = -1;
    QScopedPointer<DrmEventHost> m_eventHost;
};

class DrmEventReader
{
public:
    ~DrmEventReader();

    void create(DrmDevice *device);
    void destroy();

    void startWaitFlip(void *key, QMutex *mutex, QWaitCondition *cond);

private:
    QPointer<DrmDevice> m_device;
    QScopedPointer<DrmEventReaderThread> m_thread;
};

} // namespace Platform

} // namespace Aurora
