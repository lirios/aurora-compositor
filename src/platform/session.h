// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>

#include <LiriAuroraPlatform/liriauroraplatformglobal.h>

#include <memory>

namespace Aurora {

namespace Platform {

class LIRIAURORAPLATFORM_EXPORT Session : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
public:
    Q_DISABLE_COPY_MOVE(Session)

    enum class Type {
        Noop,
        Logind,
    };
    Q_ENUM(Type)

    enum class Capability : uint {
        SwitchTerminal = 0x1,
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    virtual QString name() const = 0;

    virtual Capabilities capabilities() const = 0;

    virtual bool isActive() const = 0;

    virtual QString seat() const = 0;
    virtual uint terminal() const = 0;

    virtual int openRestricted(const QString &fileName) = 0;
    virtual void closeRestricted(int fileDescriptor) = 0;

    virtual void switchTo(uint terminal) = 0;

    static Session *create(QObject *parent = nullptr);
    static Session *create(Type type, QObject *parent = nullptr);

Q_SIGNALS:
    void activeChanged(bool active);
    void awoke();
    void deviceResumed(dev_t deviceId);
    void devicePaused(dev_t deviceId);

protected:
    explicit Session(QObject *parent = nullptr);
};

} // namespace Platform

} // namespace Aurora

Q_DECLARE_OPERATORS_FOR_FLAGS(Aurora::Platform::Session::Capabilities)
