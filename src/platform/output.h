// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QImage>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QScreen>
#include <QUuid>

#include <LiriAuroraPlatform/Edid>

namespace Aurora {

namespace Platform {

class OutputPrivate;

class LIRIAURORAPLATFORM_EXPORT Output : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT)
    Q_PROPERTY(QScreen *screen READ screen NOTIFY screenChanged)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QString manufacturer READ manufacturer CONSTANT)
    Q_PROPERTY(QString model READ model CONSTANT)
    Q_PROPERTY(QString serialNumber READ serialNumber CONSTANT)
    Q_PROPERTY(QSize physicalSize READ physicalSize CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QPoint globalPosition READ globalPosition NOTIFY globalPositionChanged)
    Q_PROPERTY(QSize modeSize READ modeSize NOTIFY modeSizeChanged)
    Q_PROPERTY(QSize pixelSize READ pixelSize NOTIFY pixelSizeChanged)
    Q_PROPERTY(qreal scale READ scale NOTIFY scaleChanged)
    Q_PROPERTY(QRect geometry READ geometry NOTIFY geometryChanged)
    Q_PROPERTY(int refreshRate READ refreshRate NOTIFY refreshRateChanged)
    Q_PROPERTY(int depth READ depth CONSTANT)
    Q_PROPERTY(int format READ format CONSTANT)
    Q_PROPERTY(PowerState powerState READ powerState WRITE setPowerState NOTIFY powerStateChanged)
    Q_PROPERTY(Subpixel subpixel READ subpixel CONSTANT)
    Q_PROPERTY(Transform transform READ transform NOTIFY transformChanged)
    Q_PROPERTY(
            ContentType contenType READ contentType WRITE setContentType NOTIFY contentTypeChanged)
    Q_DECLARE_PRIVATE(Output)
public:
    Q_DISABLE_COPY_MOVE(Output)

    enum class Capability : uint {
        PowerState = 1,
        Overscan = 1 << 1,
        Vrr = 1 << 2,
        RgbRange = 1 << 3,
        HighDynamicRange = 1 << 4,
        WideColorGamut = 1 << 5,
        AutoRotation = 1 << 6,
        IccProfile = 1 << 7,
        Tearing = 1 << 8,
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    enum class PowerState {
        On,
        Standby,
        Suspend,
        Off,
    };
    Q_ENUM(PowerState)

    enum class Subpixel {
        Unknown,
        None,
        HorizontalRGB,
        HorizontalBGR,
        VerticalRGB,
        VerticalBGR,
    };
    Q_ENUM(Subpixel)

    enum class Transform {
        Normal,
        Rotated90,
        Rotated180,
        Rotated270,
        Flipped,
        Flipped90,
        Flipped180,
        Flipped270,
    };
    Q_ENUM(Transform);

    enum class ContentType {
        Unknown,
        Photo,
        Video,
        Game,
    };
    Q_ENUM(ContentType)

    enum class RgbRange {
        Automatic,
        Full,
        Limited,
    };
    Q_ENUM(RgbRange)

    enum class AutoRotationPolicy {
        Never,
        InTabletMode,
        Always
    };
    Q_ENUM(AutoRotationPolicy)

    struct Mode
    {
        enum class Flag {
            None = 0,
            Current = 1 << 0,
            Preferred = 1 << 1,
        };
        Q_DECLARE_FLAGS(Flags, Flag)

        /*! Weather this mode is current or preferred */
        Flags flags = Flag::None;

        /*! Size in pixel space */
        QSize size;

        /*! Refresh rate in mHz */
        int refreshRate = 0;

        bool operator==(const Mode &m) const;
    };

    explicit Output(QObject *parent = nullptr);
    ~Output();

    QUuid uuid() const;

    QScreen *screen() const;
    void setScreen(QScreen *screen);

    QString name() const;
    QString description() const;

    QString manufacturer() const;
    QString model() const;
    QString serialNumber() const;

    QSize physicalSize() const;

    bool isEnabled() const;

    QPoint globalPosition() const;
    QSize pixelSize() const;
    QSize modeSize() const;

    qreal scale() const;

    QRect geometry() const;

    int refreshRate() const;

    int depth() const;
    QImage::Format format() const;

    QList<Mode> modes() const;

    PowerState powerState() const;
    void setPowerState(PowerState powerState);

    Subpixel subpixel() const;

    Transform transform() const;

    ContentType contentType() const;
    void setContentType(ContentType contentType);

Q_SIGNALS:
    void screenChanged(QScreen *screen);
    void enabledChanged(bool enabled);
    void globalPositionChanged(const QPoint &globalPosition);
    void modeSizeChanged(const QSize &modeSize);
    void pixelSizeChanged(const QSize &pixelSize);
    void scaleChanged(qreal scale);
    void geometryChanged(const QRect &geometry);
    void refreshRateChanged(int refreshRate);
    void powerStateChanged(Output::PowerState powerState);
    void transformChanged(Output::Transform transform);
    void contentTypeChanged(Output::ContentType contentType);
    void modeAdded(const Mode &mode);
    void modeChanged(const Mode &mode);

protected:
    QScopedPointer<OutputPrivate> const d_ptr;

    struct Information
    {
        QString name;
        QString description;
        QString manufacturer;
        QString model;
        QString serialNumber;
        QString eisaId;
        QSize physicalSize;
        Edid edid;
        Subpixel subpixel = Subpixel::Unknown;
        Capabilities capabilities;
        Transform panelOrientation = Transform::Normal;
        bool internal = false;
        bool placeholder = false;
        bool nonDesktop = false;
        QByteArray mstPath;
        std::optional<double> maxPeakBrightness;
        std::optional<double> maxAverageBrightness;
        double minBrightness = 0;
    };

    struct State
    {
        QPoint position;
        qreal scale = 1;
        Transform transform = Transform::Normal;
        Transform manualTransform = Transform::Normal;
        // QList<std::shared_ptr<OutputMode>> modes;
        // std::shared_ptr<OutputMode> currentMode;
        PowerState powerState = PowerState::On;
        Subpixel subpixel = Subpixel::Unknown;
        bool enabled = false;
        uint32_t overscan = 0;
        RgbRange rgbRange = RgbRange::Automatic;
        bool wideColorGamut = false;
        bool highDynamicRange = false;
        uint32_t sdrBrightness = 200;
        AutoRotationPolicy autoRotatePolicy = AutoRotationPolicy::InTabletMode;
        QString iccProfilePath;
        // std::shared_ptr<IccProfile> iccProfile;
        // ColorDescription colorDescription = ColorDescription::sRGB;
        std::optional<double> maxPeakBrightnessOverride;
        std::optional<double> maxAverageBrightnessOverride;
        std::optional<double> minBrightnessOverride;
        double sdrGamutWideness = 0;
        // VrrPolicy vrrPolicy = VrrPolicy::Automatic;
    };

    void setInformation(const Information &information);
    void setState(const State &state);
};

LIRIAURORAPLATFORM_EXPORT QDebug operator<<(QDebug debug, const Output *output);

typedef QList<Output *> Outputs;

Q_DECLARE_OPERATORS_FOR_FLAGS(Output::Capabilities)
Q_DECLARE_OPERATORS_FOR_FLAGS(Output::Mode::Flags)

} // namespace Platform

} // namespace Aurora

Q_DECLARE_METATYPE(Aurora::Platform::Output::PowerState)
Q_DECLARE_METATYPE(Aurora::Platform::Output::Subpixel)
Q_DECLARE_METATYPE(Aurora::Platform::Output::Transform)
Q_DECLARE_METATYPE(Aurora::Platform::Output::ContentType)
Q_DECLARE_METATYPE(Aurora::Platform::Output::Mode)
