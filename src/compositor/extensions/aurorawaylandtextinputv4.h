/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#ifndef AURORA_COMPOSITOR_WAYLANDTEXTINPUTV4_H
#define AURORA_COMPOSITOR_WAYLANDTEXTINPUTV4_H

#include <LiriAuroraCompositor/WaylandCompositorExtension>

struct wl_client;

class QInputMethodEvent;
class QKeyEvent;

namespace Aurora {

namespace Compositor {

class WaylandTextInputV4Private;

class WaylandSurface;

class WaylandTextInputV4 : public WaylandCompositorExtensionTemplate<WaylandTextInputV4>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandTextInputV4)
public:
    explicit WaylandTextInputV4(WaylandObject *container, WaylandCompositor *compositor);
    ~WaylandTextInputV4() override;

    void sendInputMethodEvent(QInputMethodEvent *event);
    void sendKeyEvent(QKeyEvent *event);

    QVariant inputMethodQuery(Qt::InputMethodQuery property, QVariant argument) const;

    WaylandSurface *focus() const;
    void setFocus(WaylandSurface *surface);

    bool isSurfaceEnabled(WaylandSurface *surface) const;

    void add(::wl_client *client, uint32_t id, int version);
    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void updateInputMethod(Qt::InputMethodQueries queries);
    void surfaceEnabled(WaylandSurface *surface);
    void surfaceDisabled(WaylandSurface *surface);

private:
    void focusSurfaceDestroyed(void *);
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDTEXTINPUTV4_H
