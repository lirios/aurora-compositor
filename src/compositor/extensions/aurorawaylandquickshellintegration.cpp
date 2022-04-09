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

#include "aurorawaylandquickshellintegration.h"

/*!
 * \class WaylandQuickShellIntegration
 * \inmodule QtWaylandCompositor
 * \since 5.14
 * \brief Provides support for shell surface integration with QtQuick.
 *
 * Shell surface implementations should inherit from this class in order to provide
 * an integration between the shell surface and QtQuick.
 *
 * Shell integration is installed as an event filter for a WaylandQuickShellSurfaceItem.
 * Reimplement the event filter method and return \c true when you want to filter the
 * event out, otherwise return \c false.
 *
 * Example:
 *
 * \code
 * class MyShellIntegration : public WaylandQuickShellIntegration
 * {
 *     Q_OBJECT
 * public:
 *     MyShellIntegration(QObject *parent = nullptr);
 *
 * protected:
 *     bool eventFilter(QObject *object, QEvent *event) override;
 * };
 *
 * MyShellIntegration::MyShellIntegration(QObject *parent)
 *     : WaylandQuickShellIntegration(parent)
 * {
 * }
 *
 * bool MyShellIntegration::eventFilter(QObject *object, QEvent *event)
 * {
 *     WaylandQuickShellSurfaceItem *shellSurfaceItem = qobject_cast<WaylandQuickShellSurfaceItem *>(object);
 *     if (!shellSurfaceItem)
 *         return WaylandQuickShellIntegration::eventFilter(object, event);
 *
 *     if (event->type() == QEvent::MouseMove) {
 *         QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
 *         qDebug() << "Mouse moved on" << shellSurfaceItem << "pos:" << mouseEvent->pos();
 *         return true;
 *     }
 *
 *     return WaylandQuickShellIntegration::eventFilter(object, event);
 * }
 * \endcode
 *
 * \sa WaylandQuickShellSurfaceItem
 * \sa QObject::eventFilter()
 */

WaylandQuickShellIntegration::WaylandQuickShellIntegration(QObject *parent)
    : QObject(parent)
{
}

WaylandQuickShellIntegration::~WaylandQuickShellIntegration()
{
}
