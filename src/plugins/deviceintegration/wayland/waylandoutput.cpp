// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <LiriAuroraPlatform/private/output_p.h>

#include "waylandbackend.h"
#include "waylandoutput.h"

namespace Aurora {

namespace Platform {

static Output::Subpixel convertSubpixel(KWayland::Client::Output::SubPixel subpixel)
{
    switch (subpixel) {
    case KWayland::Client::Output::SubPixel::None:
        return Output::Subpixel::None;
    case KWayland::Client::Output::SubPixel::HorizontalRGB:
        return Output::Subpixel::HorizontalRGB;
    case KWayland::Client::Output::SubPixel::HorizontalBGR:
        return Output::Subpixel::HorizontalBGR;
    case KWayland::Client::Output::SubPixel::VerticalRGB:
        return Output::Subpixel::VerticalRGB;
    case KWayland::Client::Output::SubPixel::VerticalBGR:
        return Output::Subpixel::VerticalBGR;
    default:
        return Output::Subpixel::Unknown;
    }
}

static Output::Transform convertTransform(KWayland::Client::Output::Transform transform)
{
    switch (transform) {
    case KWayland::Client::Output::Transform::Rotated90:
        return Output::Transform::Rotated90;
    case KWayland::Client::Output::Transform::Rotated180:
        return Output::Transform::Rotated180;
    case KWayland::Client::Output::Transform::Rotated270:
        return Output::Transform::Rotated270;
    case KWayland::Client::Output::Transform::Flipped:
        return Output::Transform::Flipped;
    case KWayland::Client::Output::Transform::Flipped90:
        return Output::Transform::Flipped90;
    case KWayland::Client::Output::Transform::Flipped180:
        return Output::Transform::Flipped180;
    case KWayland::Client::Output::Transform::Flipped270:
        return Output::Transform::Flipped270;
    default:
        return Output::Transform::Normal;
    }
}

static Output::Mode convertMode(const KWayland::Client::Output::Mode &mode)
{
    Output::Mode m;

    if (mode.flags.testFlag(KWayland::Client::Output::Mode::Flag::Preferred))
        m.flags.setFlag(Output::Mode::Flag::Preferred);
    if (mode.flags.testFlag(KWayland::Client::Output::Mode::Flag::Current))
        m.flags.setFlag(Output::Mode::Flag::Current);

    m.size = mode.size;
    m.refreshRate = mode.refreshRate;

    return m;
}

WaylandOutput::WaylandOutput(KWayland::Client::Output *output, const QString &name, QObject *parent)
    : Output(parent)
    , m_output(output)
    , m_name(name)
{
    updateInfo();

    connect(output, &KWayland::Client::Output::changed, this, &WaylandOutput::updateInfo);
    connect(output, &KWayland::Client::Output::modeChanged, this,
            &WaylandOutput::handleModeChanged);
}

WaylandOutput::~WaylandOutput()
{
    destroy();
}

void WaylandOutput::destroy()
{
    if (m_output) {
        m_output->destroy();
        m_output->deleteLater();
    }
}

void WaylandOutput::updateInfo()
{
    auto *d = OutputPrivate::get(this);

    setInformation(Information{ .name = m_name,
                                .description = m_output->description(),
                                .manufacturer = m_output->manufacturer(),
                                .model = m_output->model(),
                                .physicalSize = m_output->physicalSize(),
                                .subpixel=convertSubpixel(m_output->subPixel()),
                                .panelOrientation=convertTransform(m_output->transform()) });

    d->setGlobalPosition(m_output->globalPosition());
    d->setScale(m_output->scale());

    if (d->modes.length() == 0) {
        const auto modes = m_output->modes();
        for (const auto &mode : modes) {
            handleModeAdded(mode);
            if (mode.flags.testFlag(KWayland::Client::Output::Mode::Flag::Current))
                handleModeChanged(mode);
        }
    }
}

void WaylandOutput::handleModeAdded(const KWayland::Client::Output::Mode &mode)
{
    auto m = convertMode(mode);

    auto *d = OutputPrivate::get(this);
    d->modes.append(m);

    Q_EMIT modeAdded(m);
}

void WaylandOutput::handleModeChanged(const KWayland::Client::Output::Mode &newMode)
{
    auto *d = OutputPrivate::get(this);

    QList<Output::Mode>::iterator it;
    for (it = d->modes.begin(); it != d->modes.end(); ++it) {
        const auto mode = (*it);

        if (mode.refreshRate == newMode.refreshRate && mode.size == mode.size) {
            if (mode.flags.testFlag(Output::Mode::Flag::Preferred)
                == newMode.flags.testFlag(KWayland::Client::Output::Mode::Flag::Preferred)) {
                d->currentMode = it;
                Q_EMIT modeChanged(mode);
                Q_EMIT pixelSizeChanged(pixelSize());
                Q_EMIT modeSizeChanged(modeSize());
                Q_EMIT refreshRateChanged(refreshRate());
                Q_EMIT geometryChanged(geometry());
            }
        }
    }
}

} // namespace Platform

} // namespace Aurora
