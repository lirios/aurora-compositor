// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QDebug>

#include "output.h"
#include "output_p.h"

namespace Aurora {

namespace Platform {

/*!
    \class Output
    \inmodule AuroraCore
    \brief Generic output representation.

    The Output class represents an output.
 */

/*!
 * Constructs an Output with the given \a parent.
 */
Output::Output(QObject *parent)
    : QObject(parent)
    , d_ptr(new OutputPrivate(this))
{
}

Output::~Output()
{
}

/*!
 * \property Output::name
 * \brief A user presentable string representing the output.
 *
 * This property contains a user presentable string representing the output,
 * typycally something like "VGA1", "eDP-1", "HDMI1", etc.
 */
QString Output::name() const
{
    Q_D(const Output);
    return d->information.name;
}

/*!
 * \property Output::description
 * \brief Human readable description of the output.
 *
 * This property contains a human readable description of he output.
 */
QString Output::description() const
{
    Q_D(const Output);

    if (d->information.description.isEmpty())
        return manufacturer() + QLatin1Char(' ') + model();
    return d->information.description;
}

/*!
 * \property Output::uuid
 * \brief The unique identifier of the output.
 *
 * This property contains a unique identifier of the output.
 */
QUuid Output::uuid() const
{
    Q_D(const Output);
    return d->uuid;
}

/*!
 * \property Output::screen
 * \brief The QScreen object corresponding tothe output.
 *
 * This property indicates the QScreen associated with this output.
 */
QScreen *Output::screen() const
{
    Q_D(const Output);
    return d->screen;
}

void Output::setScreen(QScreen *screen)
{
    Q_D(Output);

    if (d->screen != screen) {
        d->screen = screen;
        Q_EMIT screenChanged(screen);
    }
}

/*!
 * \property Output::manufacturer
 * \brief The manufacturer of the screen.
 *
 * This property holds the manufacturer of the screen.
 */
QString Output::manufacturer() const
{
    Q_D(const Output);
    return d->information.manufacturer;
}

/*!
 * \property Output::model
 * \brief The model of the screen.
 *
 * This property holds the model of the screen.
 */
QString Output::model() const
{
    Q_D(const Output);
    return d->information.model;
}

/*!
 * \property Output::serialNumber
 * \brief The serial number of the screen.
 *
 * This property holds the serial number of the screen.
 */
QString Output::serialNumber() const
{
    Q_D(const Output);
    return d->information.serialNumber;
}

/*!
 * \property Output::physicalSize
 * \brief The physical size of the screen in millimiters.
 *
 * This property holds the physical size of the screen in millimiters.
 */
QSize Output::physicalSize() const
{
    Q_D(const Output);
    return d->information.physicalSize;
}

/*!
 * \property Output::enabled
 * \brief Weather the output is enable or not.
 *
 * This property holds weather the output is enabled or not.
 */
bool Output::isEnabled() const
{
    Q_D(const Output);
    return d->enabled;
}

/*!
 * \property Output::globalPosition
 * \brief Position in the global compositor space.
 *
 * This property holds the output position within the global compositor space.
 */
QPoint Output::globalPosition() const
{
    Q_D(const Output);
    return d->globalPosition;
}

/*!
 * \property Output::pixelSize
 * \brief Size.
 *
 * This property holds the output size in pixels, taking transform into account.
 *
 * \sa Output::modeSize
 * \sa Output::transform
 */
QSize Output::pixelSize() const
{
    Q_D(const Output);

    switch (d->transform) {
    case Output::Transform::Rotated90:
    case Output::Transform::Rotated270:
    case Output::Transform::Flipped90:
    case Output::Transform::Flipped270:
        return modeSize().transposed();
    default:
        break;
    }

    return modeSize();
}

/*!
 * \property Output::modeSize
 * \brief Actual resolution.
 *
 * This property holds the actual resolution of the output, without
 * being multiplied by the scale.
 *
 * \sa Output::pixelSize
 * \sa Output::scale
 */
QSize Output::modeSize() const
{
    Q_D(const Output);

    if (d->currentMode == d->modes.end())
        return QSize();
    return d->currentMode->size;
}

/*!
 * \property Output::scale
 * \brief Scale.
 *
 * This property holds the output scale.
 */
qreal Output::scale() const
{
    Q_D(const Output);
    return d->scale;
}

/*!
 * \property Output::geometry
 * \brief Geometry of the output.
 *
 * This property holds the position of the output in the compositor space
 * and the size in pixels.
 *
 * The geometry is transformed according to the output transform.
 *
 * \sa Output::transform
 * \sa Output::globalPosition
 * \sa Output::pixelSize
 * \sa Output::scale
 */
QRect Output::geometry() const
{
    Q_D(const Output);

    if (d->currentMode == d->modes.end())
        return QRect();

    auto rect = QRect(d->globalPosition, pixelSize() / d->scale);
    auto x = rect.x();
    auto y = rect.y();
    auto width = rect.width();
    auto height = rect.height();

    switch (d->transform) {
    case Output::Transform::Normal:
        return rect;
    case Output::Transform::Rotated90:
        return QRect(y, rect.left(), height, width);
    case Output::Transform::Rotated180:
        return QRect(rect.topLeft(), QSize(width, height));
    case Output::Transform::Rotated270:
        return QRect(rect.top(), x, height, width);
    case Output::Transform::Flipped:
        return QRect(x + width, y, -width, height);
    case Output::Transform::Flipped90:
        return QRect(y + height, rect.left(), -height, width);
    case Output::Transform::Flipped180:
        return QRect(rect.bottomRight(), QSize(-width, -height));
    case Output::Transform::Flipped270:
        return QRect(rect.top(), x + width, height, -width);
    }

    return QRect();
}

/*!
 * \property Output::refreshRate
 * \brief The refresh rate of the output in mHz.
 *
 * This property holds the refresh rate of the output in mHz.
 */
int Output::refreshRate() const
{
    Q_D(const Output);

    if (d->currentMode == d->modes.end())
        return 0;
    return d->currentMode->refreshRate;
}

/*!
 * \property Output::depth
 * \brief Color depth.
 *
 * This property holds the color depth of the output.
 * It must be compatible with the image format: for example if the
 * Output::format property is QImage::Format_RGB32, depth must be 32.
 *
 * \ sa Output::format
 */
int Output::depth() const
{
    Q_D(const Output);
    return d->depth;
}

/*!
 * \property Output::format
 * \brief Image format.
 *
 * This property holds the image format of the output.
 * It must be compatible with color depth: for example if the
 * Output::depth property is 32, format might be QImage::Format_RGB32.
 *
 * \sa Output::depth
 */
QImage::Format Output::format() const
{
    Q_D(const Output);
    return d->format;
}

/*!
 * \property Output::modes
 * \brief Modes
 *
 * This property holds the list of modes of the output.
 */
QList<Output::Mode> Output::modes() const
{
    Q_D(const Output);
    return d->modes;
}

/*!
 * \property Output::powerState
 * \brief The power state.
 *
 * This property holds the power state of the screen.
 */
Output::PowerState Output::powerState() const
{
    Q_D(const Output);
    return d->powerState;
}

void Output::setPowerState(Output::PowerState powerState)
{
    Q_D(Output);

    if (powerState == d->powerState)
        return;

    d->powerState = powerState;
    Q_EMIT powerStateChanged(powerState);
}

Output::Subpixel Output::subpixel() const
{
    Q_D(const Output);
    return d->information.subpixel;
}

Output::Transform Output::transform() const
{
    Q_D(const Output);
    return d->state.transform;
}

Output::ContentType Output::contentType() const
{
    Q_D(const Output);
    return d->contentType;
}

void Output::setContentType(Output::ContentType contentType)
{
    Q_D(Output);

    if (d->contentType == contentType)
        return;

    d->contentType = contentType;
    Q_EMIT contentTypeChanged(contentType);
}

void Output::setInformation(const Information &information)
{
    Q_D(Output);

    d->information = information;
    // d->uuid = generateOutputId(eisaId(), model(), serialNumber(), name());
}

void Output::setState(const State &state)
{
}

QDebug operator<<(QDebug debug, const Output *output)
{
    QDebugStateSaver saver(debug);
    debug.nospace();

    if (output) {
        debug << output->metaObject()->className() << '(' << static_cast<const void *>(output);
        debug << ", name=" << output->name();
        debug << ", geometry=" << output->geometry();
        // scale
        if (debug.verbosity() > 2) {
            debug << ", manufacturer=" << output->manufacturer();
            debug << ", model=" << output->model();
            debug << ", serialNumber=" << output->serialNumber();
        }
        debug << ')';
    } else {
        debug << "Output(0x0)";
    }

    return debug;
}

bool Output::Mode::operator==(const Output::Mode &m) const
{
    return flags == m.flags && size == m.size && refreshRate == m.refreshRate;
}

/*
 * OutputPrivate
 */

OutputPrivate::OutputPrivate(Output *self)
    : q_ptr(self)
{
}

void OutputPrivate::setGlobalPosition(const QPoint &globalPosition)
{
    Q_Q(Output);

    if (this->globalPosition != globalPosition) {
        this->globalPosition = globalPosition;
        Q_EMIT q->globalPositionChanged(globalPosition);
    }
}

void OutputPrivate::setScale(qreal scale)
{
    Q_Q(Output);

    if (this->scale != scale) {
        this->scale = scale;
        Q_EMIT q->scaleChanged(scale);
    }
}

} // namespace Platform

} // namespace Aurora
