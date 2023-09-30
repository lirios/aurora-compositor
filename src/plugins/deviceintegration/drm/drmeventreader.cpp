// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2019 The Qt Company Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>
#include <QSocketNotifier>

#include "drmdevice.h"
#include "drmeventreader.h"
#include "drmloggingcategories.h"

#include <xf86drm.h>

namespace Aurora {

namespace Platform {

static void pageFlipHandler(int fd, unsigned int sequence, unsigned int tv_sec,
                            unsigned int tv_usec, void *user_data)
{
    Q_UNUSED(fd);
    Q_UNUSED(sequence);
    Q_UNUSED(tv_sec);
    Q_UNUSED(tv_usec);

    DrmEventReaderThread *t = static_cast<DrmEventReaderThread *>(QThread::currentThread());
    t->eventHost()->handlePageFlipCompleted(user_data);
}

class RegisterWaitFlipEvent : public QEvent
{
public:
    static const QEvent::Type TYPE = QEvent::Type(QEvent::User + 1);

    RegisterWaitFlipEvent(void *key, QMutex *mutex, QWaitCondition *cond)
        : QEvent(TYPE)
        , key(key)
        , mutex(mutex)
        , cond(cond)
    {
    }

    void *key;
    QMutex *mutex;
    QWaitCondition *cond;
};

/*
 * DrmEventHost
 */

bool DrmEventHost::event(QEvent *event)
{
    if (event->type() == RegisterWaitFlipEvent::TYPE) {
        RegisterWaitFlipEvent *e = static_cast<RegisterWaitFlipEvent *>(event);
        PendingFlipWait *p = &pendingFlipWaits[0];
        PendingFlipWait *end = p + MAX_FLIPS;
        while (p < end) {
            if (!p->key) {
                p->key = e->key;
                p->mutex = e->mutex;
                p->cond = e->cond;
                updateStatus();
                return true;
            }
            ++p;
        }
        qCWarning(gLcDrm, "Cannot queue page flip wait (more than %d screens?)", MAX_FLIPS);
        e->mutex->lock();
        e->cond->wakeOne();
        e->mutex->unlock();
        return true;
    }
    return QObject::event(event);
}

void DrmEventHost::updateStatus()
{
    void **begin = &completedFlips[0];
    void **end = begin + MAX_FLIPS;

    for (int i = 0; i < MAX_FLIPS; ++i) {
        PendingFlipWait *w = pendingFlipWaits + i;
        if (!w->key)
            continue;

        void **p = begin;
        while (p < end) {
            if (*p == w->key) {
                *p = nullptr;
                w->key = nullptr;
                w->mutex->lock();
                w->cond->wakeOne();
                w->mutex->unlock();
                return;
            }
            ++p;
        }
    }
}

void DrmEventHost::handlePageFlipCompleted(void *key)
{
    void **begin = &completedFlips[0];
    void **end = begin + MAX_FLIPS;
    void **p = begin;
    while (p < end) {
        if (*p == key) {
            updateStatus();
            return;
        }
        ++p;
    }
    p = begin;
    while (p < end) {
        if (!*p) {
            *p = key;
            updateStatus();
            return;
        }
        ++p;
    }
    qCWarning(gLcDrm, "Cannot store page flip status (more than %d screens?)", MAX_FLIPS);
}

/*
 * DrmEventReaderThread
 */

DrmEventReaderThread::DrmEventReaderThread(int fd, QObject *parent)
    : QThread(parent)
    , m_fd(fd)
    , m_eventHost(new DrmEventHost())
{
}

DrmEventReaderThread::~DrmEventReaderThread()
{
}

DrmEventHost *DrmEventReaderThread::eventHost() const
{
    return m_eventHost.data();
}

void DrmEventReaderThread::run()
{
    qCDebug(gLcDrm, "Event reader thread: entering event loop");

    QSocketNotifier notifier(m_fd, QSocketNotifier::Read);
    QObject::connect(&notifier, &QSocketNotifier::activated, &notifier, [this] {
        drmEventContext drmEvent;
        memset(&drmEvent, 0, sizeof(drmEvent));
        drmEvent.version = 2;
        drmEvent.vblank_handler = nullptr;
        drmEvent.page_flip_handler = pageFlipHandler;
        drmHandleEvent(m_fd, &drmEvent);
    });

    exec();

    // Move back to the thread where m_eventHost was created
    m_eventHost->moveToThread(thread());

    qCDebug(gLcDrm, "Event reader thread: event loop stopped");
}

/*
 * DrmEventReader
 */

DrmEventReader::~DrmEventReader()
{
    destroy();
}

void DrmEventReader::create(DrmDevice *device)
{
    destroy();

    if (!device)
        return;

    m_device = device;

    qCDebug(gLcDrm, "Initalizing event reader for device \"%s\" fd %d",
            qPrintable(m_device->deviceNode()), m_device->fd());

    m_thread.reset(new DrmEventReaderThread(m_device->fd()));
    m_thread->start();

    // Change thread affinity for the event host, so that postEvent()
    // goes through the event reader thread's event loop for that object.
    m_thread->eventHost()->moveToThread(m_thread.get());
}

void DrmEventReader::destroy()
{
    if (!m_device)
        return;

    qCDebug(gLcDrm, "Stopping event reader for device \"%s\"", qPrintable(m_device->deviceNode()));

    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
        m_thread.reset();
    }

    m_device.clear();
}

void DrmEventReader::startWaitFlip(void *key, QMutex *mutex, QWaitCondition *cond)
{
    if (m_thread) {
        QCoreApplication::postEvent(m_thread->eventHost(),
                                    new RegisterWaitFlipEvent(key, mutex, cond));
    }
}

} // namespace Platform

} // namespace Aurora
