// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <LiriAuroraPlatform/Output>

#include <KWayland/Client/surface.h>

#include "waylandbackend.h"
#include "waylandcursor.h"
#include "waylandcursor.h"
#include "waylandoutput.h"
#include "waylandpointer.h"
#include "waylandwindow.h"

#include <linux/input-event-codes.h>

static const QHash<quint32, Qt::MouseButton> s_buttonToQtMouseButton = {
    { BTN_LEFT, Qt::LeftButton },       { BTN_MIDDLE, Qt::MiddleButton },
    { BTN_RIGHT, Qt::RightButton },     { BTN_SIDE, Qt::ExtraButton1 },
    { BTN_EXTRA, Qt::ExtraButton2 },    { BTN_BACK, Qt::BackButton },
    { BTN_FORWARD, Qt::ForwardButton }, { BTN_TASK, Qt::TaskButton },
    { 0x118, Qt::ExtraButton6 },        { 0x119, Qt::ExtraButton7 },
    { 0x11a, Qt::ExtraButton8 },        { 0x11b, Qt::ExtraButton9 },
    { 0x11c, Qt::ExtraButton10 },       { 0x11d, Qt::ExtraButton11 },
    { 0x11e, Qt::ExtraButton12 },       { 0x11f, Qt::ExtraButton13 },
};

namespace Aurora {

namespace Platform {

WaylandPointer::WaylandPointer(KWayland::Client::Seat *hostSeat, QObject *parent)
    : PointerDevice(parent)
    , m_hostSeat(hostSeat)
    , m_hostPointer(hostSeat->createPointer(this))
{
    connect(m_hostPointer, &KWayland::Client::Pointer::entered, this,
            [this](quint32, const QPointF &) {
                m_enteredSurface = m_hostPointer->enteredSurface();

                auto *wlWindow =
                        WaylandBackend::instance()->findWindow(m_hostPointer->enteredSurface());
                if (wlWindow) {
                    wlWindow->cursor()->setPointer(m_hostPointer);
                    wlWindow->cursor()->setEnabled(true);
                }
            });
    connect(m_hostPointer, &KWayland::Client::Pointer::left, this, [this](quint32) {
        if (!m_enteredSurface.isNull()) {
            auto *wlWindow = WaylandBackend::instance()->findWindow(m_enteredSurface.data());
            if (wlWindow) {
                wlWindow->cursor()->setPointer(nullptr);
                wlWindow->cursor()->setEnabled(false);
            }

            m_enteredSurface.clear();
        }
    });
    connect(m_hostPointer, &KWayland::Client::Pointer::motion, this,
            [this](const QPointF &relativeToSurface, quint32) {
                if (!m_enteredSurface.isNull()) {
                    auto *wlWindow =
                            WaylandBackend::instance()->findWindow(m_enteredSurface.data());
                    if (wlWindow) {
                        auto absPos = wlWindow->output()->globalPosition() + relativeToSurface;
                        Q_EMIT motion(absPos);
                    }
                }
            });
    connect(m_hostPointer, &KWayland::Client::Pointer::buttonStateChanged, this,
            [this](quint32, quint32, quint32 button, KWayland::Client::Pointer::ButtonState state) {
                Qt::MouseButton mouseButton = buttonToQtMouseButton(button);

                if (state == KWayland::Client::Pointer::ButtonState::Pressed)
                    Q_EMIT buttonPressed(mouseButton);
                else if (state == KWayland::Client::Pointer::ButtonState::Released)
                    Q_EMIT buttonReleased(mouseButton);
            });
}

QString WaylandPointer::seatName() const
{
    return m_hostSeat->name();
}

Qt::MouseButton WaylandPointer::buttonToQtMouseButton(quint32 button)
{
    return s_buttonToQtMouseButton.value(button, Qt::MaxMouseButton);
}

} // namespace Platform

} // namespace Aurora
