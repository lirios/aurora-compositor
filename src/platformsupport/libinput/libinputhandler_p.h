/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2015-2016 Pier Luigi Fiorini
 *
 * Author(s):
 *    Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef LIRI_LIBINPUTHANDLER_P_H
#define LIRI_LIBINPUTHANDLER_P_H

#include <QtCore/private/qobject_p.h>

#include <LiriAuroraLibInput/liriauroralibinputglobal.h>

#include <LiriAuroraUdev/Udev>

#include <LiriAuroraLibInput/libinputgesture.h>
#include <LiriAuroraLibInput/libinputkeyboard.h>
#include <LiriAuroraLibInput/libinputpointer.h>
#include <LiriAuroraLibInput/libinputtouch.h>

#include <libinput.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Liri LibInput API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace Liri {

namespace Platform {

class LIRIAURORALIBINPUT_EXPORT LibInputHandlerPrivate
{
    Q_DECLARE_PUBLIC(LibInputHandler)
public:
    LibInputHandlerPrivate(LibInputHandler *self);
    ~LibInputHandlerPrivate();

    void setup();
    void initialize();

    static void logHandler(libinput *handle, libinput_log_priority priority,
                           const char *format, va_list args);

    static int restrictedOpenCallback(const char *path, int flags, void *user_data);
    static void restrictedCloseCallback(int fd, void *user_data);

    bool initialized;

    QtUdev::Udev *udev;
    libinput *li;

    LibInputKeyboard *keyboard;
    int keyboardCount;

    LibInputPointer *pointer;
    int pointerCount;

    LibInputTouch *touch;
    int touchCount;

    int tabletCount;

    LibInputGesture *gesture;
    int gestureCount;

    bool suspended;

    static const struct libinput_interface liInterface;

private:
    LibInputHandler *q_ptr = nullptr;

    int restrictedOpen(const char *path, int flags);
    void restrictedClose(int fd);
};

} // namespace Platform

} // namespace Liri

#endif // LIRI_LIBINPUTHANDLER_P_H
