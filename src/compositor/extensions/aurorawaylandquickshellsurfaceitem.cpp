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

#include "aurorawaylandquickshellsurfaceitem.h"
#include "aurorawaylandquickshellsurfaceitem_p.h"

#include <LiriAuroraCompositor/WaylandShellSurface>
#include <QGuiApplication>

namespace Aurora {

namespace Compositor {

WaylandQuickShellSurfaceItem *WaylandQuickShellSurfaceItemPrivate::maybeCreateAutoPopup(WaylandShellSurface* shellSurface)
{
    if (!m_autoCreatePopupItems)
        return nullptr;

    Q_Q(WaylandQuickShellSurfaceItem);
    auto *popupItem = new WaylandQuickShellSurfaceItem(q);
    popupItem->setShellSurface(shellSurface);
    popupItem->setAutoCreatePopupItems(true);
    QObject::connect(popupItem, &WaylandQuickShellSurfaceItem::surfaceDestroyed, [popupItem](){
        popupItem->deleteLater();
    });
    return popupItem;
}

/*!
 * \qmltype ShellSurfaceItem
 * \instantiates WaylandQuickShellSurfaceItem
 * \inherits WaylandQuickItem
 * \inqmlmodule Aurora.Compositor
 * \since 5.8
 * \brief A Qt Quick item type for displaying and interacting with a ShellSurface.
 *
 * This type is used to render \c wl_shell, \c xdg_shell or \c ivi_application surfaces as part of
 * a Qt Quick scene. It handles moving and resizing triggered by clicking on the window decorations.
 *
 * \sa WaylandQuickItem, WlShellSurface, IviSurface
 */

/*!
 * \class WaylandQuickShellSurfaceItem
 * \inmodule QtWaylandCompositor
 * \since 5.8
 * \brief The WaylandQuickShellSurfaceItem class provides a Qt Quick item that represents a WaylandShellSurface.
 *
 * This class is used to render \c wl_shell, \c xdg_shell or \c ivi_application surfaces as part of
 * a Qt Quick scene. It handles moving and resizing triggered by clicking on the window decorations.
 *
 * \sa WaylandQuickItem, WaylandWlShellSurface, WaylandIviSurface
 */

/*!
 * Constructs a WaylandQuickWlShellSurfaceItem with the given \a parent.
 */
WaylandQuickShellSurfaceItem::WaylandQuickShellSurfaceItem(QQuickItem *parent)
    : WaylandQuickItem(*new WaylandQuickShellSurfaceItemPrivate(), parent)
{
}

WaylandQuickShellSurfaceItem::~WaylandQuickShellSurfaceItem()
{
    Q_D(WaylandQuickShellSurfaceItem);

    if (d->m_shellIntegration) {
        removeEventFilter(d->m_shellIntegration);
        delete d->m_shellIntegration;
    }
}

/*!
 * \internal
 */
WaylandQuickShellSurfaceItem::WaylandQuickShellSurfaceItem(WaylandQuickShellSurfaceItemPrivate &dd, QQuickItem *parent)
    : WaylandQuickItem(dd, parent)
{
}

/*!
 * \qmlproperty ShellSurface QtWaylandCompositor::ShellSurfaceItem::shellSurface
 *
 * This property holds the ShellSurface rendered by this ShellSurfaceItem.
 * It may either be an XdgSurfaceV5, WlShellSurface or IviSurface depending on which shell protocol
 * is in use.
 */

/*!
 * \property WaylandQuickShellSurfaceItem::shellSurface
 *
 * This property holds the WaylandShellSurface rendered by this WaylandQuickShellSurfaceItem.
 * It may either be a WaylandXdgSurfaceV5, WaylandWlShellSurface or WaylandIviSurface depending
 * on which shell protocol is in use.
 */
WaylandShellSurface *WaylandQuickShellSurfaceItem::shellSurface() const
{
    Q_D(const WaylandQuickShellSurfaceItem);
    return d->m_shellSurface;
}

void WaylandQuickShellSurfaceItem::setShellSurface(WaylandShellSurface *shellSurface)
{
    Q_D(WaylandQuickShellSurfaceItem);
    if (d->m_shellSurface == shellSurface)
        return;

    d->m_shellSurface = shellSurface;

    if (d->m_shellIntegration) {
        removeEventFilter(d->m_shellIntegration);
        delete d->m_shellIntegration;
        d->m_shellIntegration = nullptr;
    }

    if (shellSurface) {
        d->m_shellIntegration = shellSurface->createIntegration(this);
        installEventFilter(d->m_shellIntegration);
    }

    emit shellSurfaceChanged();
}

/*!
 * \qmlproperty Item QtWaylandCompositor::ShellSurfaceItem::moveItem
 *
 * This property holds the move item for this ShellSurfaceItem. This is the item that will be moved
 * when the clients request the ShellSurface to be moved, maximized, resized etc. This property is
 * useful when implementing server-side decorations.
 */

/*!
 * \property WaylandQuickShellSurfaceItem::moveItem
 *
 * This property holds the move item for this WaylandQuickShellSurfaceItem. This is the item that
 * will be moved when the clients request the WaylandShellSurface to be moved, maximized, resized
 * etc. This property is useful when implementing server-side decorations.
 */
QQuickItem *WaylandQuickShellSurfaceItem::moveItem() const
{
    Q_D(const WaylandQuickShellSurfaceItem);
    return d->m_moveItem ? d->m_moveItem : const_cast<WaylandQuickShellSurfaceItem *>(this);
}

void WaylandQuickShellSurfaceItem::setMoveItem(QQuickItem *moveItem)
{
    Q_D(WaylandQuickShellSurfaceItem);
    moveItem = moveItem ? moveItem : this;
    if (this->moveItem() == moveItem)
        return;
    d->m_moveItem = moveItem;
    emit moveItemChanged();
}

/*!
 * \qmlproperty bool QtWaylandCompositor::ShellSurfaceItem::autoCreatePopupItems
 *
 * This property holds whether ShellSurfaceItems for popups parented to the shell
 * surface managed by this item should automatically be created.
 */

/*!
 * \property WaylandQuickShellSurfaceItem::autoCreatePopupItems
 *
 * This property holds whether WaylandQuickShellSurfaceItems for popups
 * parented to the shell surface managed by this item should automatically be created.
 */
bool WaylandQuickShellSurfaceItem::autoCreatePopupItems()
{
    Q_D(const WaylandQuickShellSurfaceItem);
    return d->m_autoCreatePopupItems;
}

void WaylandQuickShellSurfaceItem::setAutoCreatePopupItems(bool enabled)
{
    Q_D(WaylandQuickShellSurfaceItem);

    if (enabled == d->m_autoCreatePopupItems)
        return;

    d->m_autoCreatePopupItems = enabled;
    emit autoCreatePopupItemsChanged();
}

/*!
\class WaylandQuickShellEventFilter
\brief WaylandQuickShellEventFilter implements a Wayland popup grab
\internal
*/

void WaylandQuickShellEventFilter::startFilter(WaylandClient *client, CallbackFunction closePopups)
{
    if (!self)
        self = new WaylandQuickShellEventFilter(qGuiApp);
    if (!self->eventFilterInstalled) {
        qGuiApp->installEventFilter(self);
        self->eventFilterInstalled = true;
        self->client = client;
        self->closePopups = closePopups;
    }
}

void WaylandQuickShellEventFilter::cancelFilter()
{
    if (!self)
        return;
    if (self->eventFilterInstalled && !self->waitForRelease)
        self->stopFilter();
}

void WaylandQuickShellEventFilter::stopFilter()
{
    if (eventFilterInstalled) {
        qGuiApp->removeEventFilter(this);
        eventFilterInstalled = false;
    }
}
WaylandQuickShellEventFilter *WaylandQuickShellEventFilter::self = nullptr;

WaylandQuickShellEventFilter::WaylandQuickShellEventFilter(QObject *parent)
    : QObject(parent)
{
}

bool WaylandQuickShellEventFilter::eventFilter(QObject *receiver, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease) {
        bool press = e->type() == QEvent::MouseButtonPress;
        if (press && !waitForRelease) {
            // The user clicked something: we need to close popups unless this press is caught later
            if (!mousePressTimeout.isActive())
                mousePressTimeout.start(0, this);
        }

        QQuickItem *item = qobject_cast<QQuickItem*>(receiver);
        if (!item)
            return false;

        QMouseEvent *event = static_cast<QMouseEvent*>(e);
        WaylandQuickShellSurfaceItem *shellSurfaceItem = qobject_cast<WaylandQuickShellSurfaceItem*>(item);
        bool finalRelease = (event->type() == QEvent::MouseButtonRelease) && (event->buttons() == Qt::NoButton);
        bool popupClient = shellSurfaceItem && shellSurfaceItem->surface() && shellSurfaceItem->surface()->client() == client;

        if (waitForRelease) {
            // We are eating events until all mouse buttons are released
            if (finalRelease) {
                waitForRelease = false;
                stopFilter();
            }
            return true;
        }

        if (finalRelease && mousePressTimeout.isActive()) {
            // the user somehow managed to press and release the mouse button in 0 milliseconds
            qWarning("Badly written autotest detected");
            mousePressTimeout.stop();
            stopFilter();
        }

        if (press && !shellSurfaceItem && !QQmlProperty(item, QStringLiteral("qtwayland_blocking_overlay")).isValid()) {
            // the user clicked on something that's not blocking mouse events
            e->ignore(); //propagate the event to items below
            return true; // don't give the event to the item
        }

        mousePressTimeout.stop(); // we've got this

        if (press && !popupClient) {
            // The user clicked outside the active popup's client. The popups should
            // be closed, but the event filter will stay to catch the release-
            // event before removing itself.
            waitForRelease = true;
            closePopups();
            return true;
        }
    }

    return false;
}

void WaylandQuickShellEventFilter::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mousePressTimeout.timerId()) {
        mousePressTimeout.stop();
        closePopups();
        stopFilter();
        // Don't wait for release: Since the press wasn't accepted,
        // the release won't be delivered.
    }
}

static WaylandQuickShellSurfaceItem *findSurfaceItemFromMoveItem(QQuickItem *moveItem)
{
    if (Q_UNLIKELY(!moveItem))
        return nullptr;
    if (auto *surf = qobject_cast<WaylandQuickShellSurfaceItem *>(moveItem))
        return surf;
    const auto children = moveItem->childItems();
    for (auto *item : children) {
        if (auto *surf = findSurfaceItemFromMoveItem(item))
            return surf;
    }
    return nullptr;
}

/*
    To raise a surface, find the topmost suitable surface and place above that.
    We start from the top and:
    If we don't have staysOnTop, skip all surfaces with staysOnTop
    If we have staysOnBottom, skip all surfaces that don't have staysOnBottom
  */
void WaylandQuickShellSurfaceItemPrivate::raise()
{
    Q_Q(WaylandQuickShellSurfaceItem);
    auto *moveItem = q->moveItem();
    QQuickItem *parent = moveItem->parentItem();
    if (!parent)
        return;
    auto it = parent->childItems().crbegin();
    auto skip = [this](QQuickItem *item) {
        if (auto *surf = findSurfaceItemFromMoveItem(item))
            return (!staysOnTop && surf->staysOnTop()) || (staysOnBottom && !surf->staysOnBottom());
        return true; // ignore any other Quick items that may be there
    };
    while (skip(*it))
        ++it;
    QQuickItem *top = *it;
    if (moveItem != top)
        moveItem->stackAfter(top);
}

/*
    To lower a surface, find the lowest suitable surface and place below that.
    We start from the bottom and:
    If we don't have staysOnBottom, skip all surfaces with staysOnBottom
    If we have staysOnTop, skip all surfaces that don't have staysOnTop
  */
void WaylandQuickShellSurfaceItemPrivate::lower()
{
    Q_Q(WaylandQuickShellSurfaceItem);
    auto *moveItem = q->moveItem();
    QQuickItem *parent = moveItem->parentItem();
    if (!parent)
        return;
    const auto children = parent->childItems();
    auto it = children.cbegin();

    auto skip = [this](QQuickItem *item) {
        if (auto *surf = findSurfaceItemFromMoveItem(item))
            return (!staysOnBottom && surf->staysOnBottom()) || (staysOnTop && !surf->staysOnTop());
        return true; // ignore any other Quick items that may be there
    };
    while (skip(*it))
        ++it;

    QQuickItem *bottom = *it;
    if (moveItem != bottom)
        moveItem->stackBefore(bottom);
}

/*!
 * \property WaylandQuickShellSurfaceItem::staysOnTop
 *
 * Keep this item above other Wayland surfaces
 */
bool WaylandQuickShellSurfaceItem::staysOnTop() const
{
    Q_D(const WaylandQuickShellSurfaceItem);
    return d->staysOnTop;
}

void WaylandQuickShellSurfaceItem::setStaysOnTop(bool onTop)
{
    Q_D(WaylandQuickShellSurfaceItem);
    if (d->staysOnTop == onTop)
        return;
    d->staysOnTop = onTop;
    if (d->staysOnBottom) {
        d->staysOnBottom = false;
        emit staysOnBottomChanged();
    }
    // We need to call raise() even if onTop is false, since we need to stack under any other
    // staysOnTop surfaces in that case
    raise();
    emit staysOnTopChanged();
    Q_ASSERT(!(d->staysOnTop && d->staysOnBottom));
}

/*!
 * \property WaylandQuickShellSurfaceItem::staysOnBottom
 *
 * Keep this item above other Wayland surfaces
 */
bool WaylandQuickShellSurfaceItem::staysOnBottom() const
{
    Q_D(const WaylandQuickShellSurfaceItem);
    return d->staysOnBottom;
}

void WaylandQuickShellSurfaceItem::setStaysOnBottom(bool onBottom)
{
    Q_D(WaylandQuickShellSurfaceItem);
    if (d->staysOnBottom == onBottom)
        return;
    d->staysOnBottom = onBottom;
    if (d->staysOnTop) {
        d->staysOnTop = false;
        emit staysOnTopChanged();
    }
    // We need to call lower() even if onBottom is false, since we need to stack over any other
    // staysOnBottom surfaces in that case
    lower();
    emit staysOnBottomChanged();
    Q_ASSERT(!(d->staysOnTop && d->staysOnBottom));
}

} // namespace Compositor

} // namespace Aurora
