/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2016 Pelagicore AG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qeglfskmsdevice.h"
#include "qeglfskmsintegration.h"
#include "qeglfskmsscreen.h"
#include "private/qeglfsintegration_p.h"
#include <QtGui/private/qguiapplication_p.h>

QT_BEGIN_NAMESPACE

QEglFSKmsDevice::QEglFSKmsDevice(KmsScreenConfig *screenConfig, const QString &path)
    : KmsDevice(screenConfig, path)
{
}

void QEglFSKmsDevice::registerScreen(QPlatformScreen *screen,
                                     bool isPrimary,
                                     const QPoint &virtualPos,
                                     const QList<QPlatformScreen *> &virtualSiblings)
{
    QEglFSKmsScreen *s = static_cast<QEglFSKmsScreen *>(screen);
    s->setVirtualPosition(virtualPos);
    s->setVirtualSiblings(virtualSiblings);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 3)
    QWindowSystemInterface::handleScreenAdded(s, isPrimary);
#else
    static_cast<QEglFSIntegration *>(QGuiApplicationPrivate::platformIntegration())->addScreen(s, isPrimary);
#endif

}

QT_END_NAMESPACE
