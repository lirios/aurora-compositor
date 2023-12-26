// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawaylandquickshellintegration.h"

/*!
 * \class WaylandQuickShellIntegration
 * \inmodule AuroraCompositor
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

namespace Aurora {

namespace Compositor {

WaylandQuickShellIntegration::WaylandQuickShellIntegration(QObject *parent)
    : QObject(parent)
{
}

WaylandQuickShellIntegration::~WaylandQuickShellIntegration()
{
}

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawaylandquickshellintegration.cpp"
