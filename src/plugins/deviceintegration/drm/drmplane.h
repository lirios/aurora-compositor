// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2016 Roman Gilg <subdiff@gmail.com>
// SPDX-FileCopyrightText: 2022 Xaver Hugl <xaver.hugl@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "drmobject.h"

#include <QMap>
#include <QPoint>
#include <QSize>
#include <memory>
#include <qobjectdefs.h>

namespace Aurora {

namespace Platform {

#if 0
class DrmFramebuffer;
#endif
class DrmCrtc;

class DrmPlane : public DrmObject
{
    Q_GADGET
public:
    DrmPlane(DrmDevice *device, uint32_t planeId);

    bool updateProperties() override;
#if 0
    void disable(DrmAtomicCommit *commit) override;
#endif

    bool isCrtcSupported(int pipeIndex) const;
    QMap<uint32_t, QList<uint64_t>> formats() const;

#if 0
    std::shared_ptr<DrmFramebuffer> currentBuffer() const;
    void setCurrentBuffer(const std::shared_ptr<DrmFramebuffer> &b);
    void releaseCurrentBuffer();

    void set(DrmAtomicCommit *commit, const QPoint &srcPos, const QSize &srcSize, const QRect &dst);
#endif

    enum class TypeIndex : uint64_t {
        Overlay = 0,
        Primary = 1,
        Cursor = 2
    };
    enum class Transformation : uint32_t {
        Rotate0 = 1 << 0,
        Rotate90 = 1 << 1,
        Rotate180 = 1 << 2,
        Rotate270 = 1 << 3,
        ReflectX = 1 << 4,
        ReflectY = 1 << 5
    };
    Q_ENUM(Transformation)
    Q_DECLARE_FLAGS(Transformations, Transformation)
    enum class PixelBlendMode : uint64_t {
        None,
        PreMultiplied,
        Coverage
    };
    enum class ColorEncoding : uint64_t {
        BT601_YCbCr,
        BT709_YCbCr,
        BT2020_YCbCr
    };
    enum class ColorRange : uint64_t {
        Limited_YCbCr,
        Full_YCbCr
    };

    DrmEnumProperty<TypeIndex> type;
    DrmProperty srcX;
    DrmProperty srcY;
    DrmProperty srcW;
    DrmProperty srcH;
    DrmProperty crtcX;
    DrmProperty crtcY;
    DrmProperty crtcW;
    DrmProperty crtcH;
    DrmProperty fbId;
    DrmProperty crtcId;
    DrmEnumProperty<Transformations> rotation;
    DrmProperty inFormats;
    DrmProperty alpha;
    DrmEnumProperty<PixelBlendMode> pixelBlendMode;
    DrmEnumProperty<ColorEncoding> colorEncoding;
    DrmEnumProperty<ColorRange> colorRange;
    DrmProperty vmHotspotX;
    DrmProperty vmHotspotY;
    DrmProperty inFenceFd;

    static int32_t transformationToDegrees(Transformations transformation);

private:
#if 0
    std::shared_ptr<DrmFramebuffer> m_current;
#endif

    QMap<uint32_t, QList<uint64_t>> m_supportedFormats;
    uint32_t m_possibleCrtcs;
};

} // namespace Platform

} // namespace Aurora

Q_DECLARE_OPERATORS_FOR_FLAGS(Aurora::Platform::DrmPlane::Transformations)
