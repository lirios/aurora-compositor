// Copyright (C) 2017 The Qt Company Ltd.
// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QtCore/QObject>

#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/WaylandSurface>

namespace Aurora {

namespace Compositor {

class WaylandKeyboard;
class WaylandKeyboardPrivate;
class WaylandSeat;
class WaylandKeymap;

class LIRIAURORACOMPOSITOR_EXPORT WaylandKeyboard : public WaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandKeyboard)
    Q_PROPERTY(quint32 repeatRate READ repeatRate WRITE setRepeatRate NOTIFY repeatRateChanged)
    Q_PROPERTY(quint32 repeatDelay READ repeatDelay WRITE setRepeatDelay NOTIFY repeatDelayChanged)
public:
    WaylandKeyboard(WaylandSeat *seat, QObject *parent = nullptr);

    WaylandSeat *seat() const;
    WaylandCompositor *compositor() const;

    quint32 repeatRate() const;
    void setRepeatRate(quint32 rate);

    quint32 repeatDelay() const;
    void setRepeatDelay(quint32 delay);

    virtual void setFocus(WaylandSurface *surface);

    virtual void sendKeyModifiers(WaylandClient *client, uint32_t serial);
    virtual void sendKeyPressEvent(uint code);
    virtual void sendKeyReleaseEvent(uint code);

    WaylandSurface *focus() const;
    WaylandClient *focusClient() const;

    virtual void addClient(WaylandClient *client, uint32_t id, uint32_t version);

    uint keyToScanCode(int qtKey) const;

Q_SIGNALS:
    void focusChanged(Aurora::Compositor::WaylandSurface *surface);
    void repeatRateChanged(quint32 repeatRate);
    void repeatDelayChanged(quint32 repeatDelay);

private:
    void focusDestroyed(void *data);

private Q_SLOTS:
    void updateKeymap();
};

} // namespace Compositor

} // namespace Aurora

