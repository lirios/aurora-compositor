/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef XCOMPOSITEHANDLER_H
#define XCOMPOSITEHANDLER_H

#include <QtWaylandCompositor/QWaylandCompositor>

#include "xlibinclude.h"

#include "qwayland-server-xcomposite.h"
#include <wayland-server-core.h>

QT_BEGIN_NAMESPACE

class XCompositeHandler : public QtWaylandServer::qt_xcomposite
{
public:
    XCompositeHandler(QWaylandCompositor *compositor, Display *display);
    bool isXCompositeBuffer(wl_resource *resource) { return mKnownBuffers.contains(resource); }
    void removeBuffer(wl_resource *resource) { mKnownBuffers.remove(resource); }

private:
    QWindow *mFakeRootWindow = nullptr;
    QString mDisplayString;
    QSet<wl_resource *> mKnownBuffers;

    void xcomposite_bind_resource(Resource *resource) override;
    void xcomposite_create_buffer(Resource *resource, uint32_t id, uint32_t x_window,
                                  int32_t width, int32_t height) override;
};

QT_END_NAMESPACE

#endif // XCOMPOSITEHANDLER_H
