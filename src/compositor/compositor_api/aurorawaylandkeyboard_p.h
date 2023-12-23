// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
#include <LiriAuroraCompositor/private/aurorawaylandcompositor_p.h>

#include <LiriAuroraCompositor/aurorawaylandseat.h>
#include <LiriAuroraCompositor/aurorawaylandkeyboard.h>
#include <LiriAuroraCompositor/aurorawaylanddestroylistener.h>

#include <LiriAuroraCompositor/private/aurora-server-wayland.h>

#include <QtCore/QList>

#if LIRI_FEATURE_aurora_xkbcommon
#include <xkbcommon/xkbcommon.h>
#include <LiriAuroraXkbCommonSupport/private/auroraxkbcommon_p.h>
#endif


namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandKeyboardPrivate : public PrivateServer::wl_keyboard
{
public:
    Q_DECLARE_PUBLIC(WaylandKeyboard)

    static WaylandKeyboardPrivate *get(WaylandKeyboard *keyboard);

    WaylandKeyboardPrivate(WaylandKeyboard *self, WaylandSeat *seat);
    ~WaylandKeyboardPrivate() override;

    WaylandCompositor *compositor() const { return seat->compositor(); }

    void focused(WaylandSurface* surface);

#if LIRI_FEATURE_aurora_xkbcommon
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
#if LIRI_FEATURE_aurora_xkbcommon
    void createXKBKeymap();
    void createXKBState(xkb_keymap *keymap);
#endif
    static uint toWaylandKey(const uint nativeScanCode);
    static uint fromWaylandKey(const uint key);

    void sendRepeatInfo();

    WaylandKeyboard *q_ptr = nullptr;

    WaylandSeat *seat = nullptr;

    WaylandSurface *focus = nullptr;
    Resource *focusResource = nullptr;
    WaylandDestroyListener focusDestroyListener;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QList<uint32_t> keys;
#else
    QVector<uint32_t> keys;
#endif
    uint32_t modsDepressed = 0;
    uint32_t modsLatched = 0;
    uint32_t modsLocked = 0;
    uint32_t group = 0;

    uint32_t shiftIndex = 0;
    uint32_t controlIndex = 0;
    uint32_t altIndex = 0;

    Qt::KeyboardModifiers currentModifierState;

    bool pendingKeymap = false;
#if LIRI_FEATURE_aurora_xkbcommon
    size_t keymap_size;
    int keymap_fd = -1;
    char *keymap_area = nullptr;
    using ScanCodeKey = std::pair<uint,int>; // group/layout and QtKey
    QMap<ScanCodeKey, uint> scanCodesByQtKey;
    XkbCommon::ScopedXKBState mXkbState;
#endif

    quint32 repeatRate = 40;
    quint32 repeatDelay = 400;
};

} // namespace Compositor

} // namespace Aurora

