// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Aurora API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QSet>
#include <QTimer>

#include <LiriAuroraGlobalPrivate/aurorafeatures.h>
#if LIRI_FEATURE_aurora_xkbcommon
#  include <LiriAuroraXkbCommonSupport/private/auroraxkbcommon_p.h>
#endif
#include <LiriAuroraPlatform/liriauroraplatformglobal.h>

namespace Aurora {

namespace Platform {

class KeyboardDevice;

class LIRIAURORAPLATFORM_EXPORT KeyboardDevicePrivate
{
    Q_DECLARE_PUBLIC(KeyboardDevice)
public:
    explicit KeyboardDevicePrivate(KeyboardDevice *self);

#if LIRI_FEATURE_aurora_xkbcommon
    bool createDefaultKeymap();

    PlatformSupport::XkbCommon::ScopedXKBContext xkbContext;
    PlatformSupport::XkbCommon::ScopedXKBKeymap xkbKeymap;
    PlatformSupport::XkbCommon::ScopedXKBState xkbState;
#endif

    quint32 nativeModifiers = 0;

    bool keyRepeatEnabled = false;
    qint32 keyRepeatRate = 25;
    qint32 keyRepeatDelay = 400;

    struct RepeatKey
    {
        int key = 0;
        quint32 code = 0;
        quint32 time = 0;
        QString text;
        Qt::KeyboardModifiers modifiers = Qt::NoModifier;
        quint32 nativeVirtualKey = 0;
        quint32 nativeModifiers = 0;
        ushort repeatCount;
    } repeatKey;
    QTimer repeatTimer;

protected:
    KeyboardDevice *q_ptr = nullptr;
};

} // namespace Platform

} // namespace Aurora