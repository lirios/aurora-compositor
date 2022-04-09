/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
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

#ifndef AURORA_WAYLAND_KEYBOARD_P_H
#define AURORA_WAYLAND_KEYBOARD_P_H

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
#include <LiriAuroraCompositor/private/aurorawaylandcompositor_p.h>

#include <LiriAuroraCompositor/private/qtwaylandcompositorglobal_p.h>
#include <LiriAuroraCompositor/aurorawaylandseat.h>
#include <LiriAuroraCompositor/aurorawaylandkeyboard.h>
#include <LiriAuroraCompositor/aurorawaylanddestroylistener.h>

#include <QtCore/private/qobject_p.h>
#include <LiriAuroraCompositor/private/aurora-server-wayland.h>

#include <QtCore/QList>

#if QT_CONFIG(xkbcommon)
#include <xkbcommon/xkbcommon.h>
#include <QtGui/private/qxkbcommon_p.h>
#endif


namespace Aurora {

namespace Compositor {

class Q_WAYLANDCOMPOSITOR_EXPORT WaylandKeyboardPrivate : public QObjectPrivate
                                                  , public PrivateServer::wl_keyboard
{
public:
    Q_DECLARE_PUBLIC(WaylandKeyboard)

    static WaylandKeyboardPrivate *get(WaylandKeyboard *keyboard);

    WaylandKeyboardPrivate(WaylandSeat *seat);
    ~WaylandKeyboardPrivate() override;

    WaylandCompositor *compositor() const { return seat->compositor(); }

    void focused(WaylandSurface* surface);

#if QT_CONFIG(xkbcommon)
    struct xkb_state *xkbState() const { return mXkbState.get(); }
    struct xkb_context *xkbContext() const {
        return WaylandCompositorPrivate::get(seat->compositor())->xkbContext();
    }
    uint32_t xkbModsMask() const { return modsDepressed | modsLatched | modsLocked; }
    void maybeUpdateXkbScanCodeTable();
    void resetKeyboardState();
#endif

    void keyEvent(uint code, uint32_t state);
    void sendKeyEvent(uint code, uint32_t state);
    void updateModifierState(uint code, uint32_t state);
    void checkAndRepairModifierState(QKeyEvent *ke);
    void maybeUpdateKeymap();

    void checkFocusResource(Resource *resource);
    void sendEnter(WaylandSurface *surface, Resource *resource);

protected:
    void keyboard_bind_resource(Resource *resource) override;
    void keyboard_destroy_resource(Resource *resource) override;
    void keyboard_release(Resource *resource) override;

private:
#if QT_CONFIG(xkbcommon)
    void createXKBKeymap();
    void createXKBState(xkb_keymap *keymap);
#endif
    static uint toWaylandKey(const uint nativeScanCode);
    static uint fromWaylandKey(const uint key);

    void sendRepeatInfo();

    WaylandSeat *seat = nullptr;

    WaylandSurface *focus = nullptr;
    Resource *focusResource = nullptr;
    WaylandDestroyListener focusDestroyListener;

    QList<uint32_t> keys;
    uint32_t modsDepressed = 0;
    uint32_t modsLatched = 0;
    uint32_t modsLocked = 0;
    uint32_t group = 0;

    uint32_t shiftIndex = 0;
    uint32_t controlIndex = 0;
    uint32_t altIndex = 0;

    Qt::KeyboardModifiers currentModifierState;

    bool pendingKeymap = false;
#if QT_CONFIG(xkbcommon)
    size_t keymap_size;
    int keymap_fd = -1;
    char *keymap_area = nullptr;
    using ScanCodeKey = std::pair<uint,int>; // group/layout and QtKey
    QMap<ScanCodeKey, uint> scanCodesByQtKey;
    QXkbCommon::ScopedXKBState mXkbState;
#endif

    quint32 repeatRate = 40;
    quint32 repeatDelay = 400;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_WAYLAND_KEYBOARD_P_H
