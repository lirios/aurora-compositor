/****************************************************************************
 * This file is part of Liri.
 *
 * Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef QTUDEV_P_H
#define QTUDEV_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtUdev.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QLoggingCategory>

#include <LiriAuroraUdev/liriauroraudevglobal.h>

extern "C" {
#include <libudev.h>
}

Q_DECLARE_LOGGING_CATEGORY(gLcUdev)

namespace Aurora {

namespace PlatformSupport {

class Udev;

class LIRIAURORAUDEV_EXPORT UdevPrivate
{
public:
    UdevPrivate();
    ~UdevPrivate();

    static UdevPrivate *get(Udev *u);

    struct udev *udev;
};

} // namespace PlatformSupport

} // namespace Aurora

#endif // QTUDEV_P_H

