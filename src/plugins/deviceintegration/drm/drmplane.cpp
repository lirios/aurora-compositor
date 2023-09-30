// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2016 Roman Gilg <subdiff@gmail.com>
// SPDX-FileCopyrightText: 2022 Xaver Hugl <xaver.hugl@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "drmdevice.h"
#include "drmplane.h"
#include "drmloggingcategories.h"
#include "drmpointer.h"

#include <drm_fourcc.h>

namespace Aurora {

namespace Platform {

DrmPlane::DrmPlane(DrmDevice *device, uint32_t planeId)
    : DrmObject(device, planeId, DRM_MODE_OBJECT_PLANE)
    , type(this, QByteArrayLiteral("type"),
           {
                   QByteArrayLiteral("Overlay"),
                   QByteArrayLiteral("Primary"),
                   QByteArrayLiteral("Cursor"),
           })
    , srcX(this, QByteArrayLiteral("SRC_X"))
    , srcY(this, QByteArrayLiteral("SRC_Y"))
    , srcW(this, QByteArrayLiteral("SRC_W"))
    , srcH(this, QByteArrayLiteral("SRC_H"))
    , crtcX(this, QByteArrayLiteral("CRTC_X"))
    , crtcY(this, QByteArrayLiteral("CRTC_Y"))
    , crtcW(this, QByteArrayLiteral("CRTC_W"))
    , crtcH(this, QByteArrayLiteral("CRTC_H"))
    , fbId(this, QByteArrayLiteral("FB_ID"))
    , crtcId(this, QByteArrayLiteral("CRTC_ID"))
    , rotation(this, QByteArrayLiteral("rotation"),
               {
                       QByteArrayLiteral("rotate-0"),
                       QByteArrayLiteral("rotate-90"),
                       QByteArrayLiteral("rotate-180"),
                       QByteArrayLiteral("rotate-270"),
                       QByteArrayLiteral("reflect-x"),
                       QByteArrayLiteral("reflect-y"),
               })
    , inFormats(this, QByteArrayLiteral("IN_FORMATS"))
    , alpha(this, QByteArrayLiteral("alpha"))
    , pixelBlendMode(this, QByteArrayLiteral("pixel blend mode"),
                     {
                             QByteArrayLiteral("None"),
                             QByteArrayLiteral("Pre-multiplied"),
                             QByteArrayLiteral("Coverage"),
                     })
    , colorEncoding(this, QByteArrayLiteral("COLOR_ENCODING"),
                    {
                            QByteArrayLiteral("ITU-R BT.601 YCbCr"),
                            QByteArrayLiteral("ITU-R BT.709 YCbCr"),
                            QByteArrayLiteral("ITU-R BT.2020 YCbCr"),
                    })
    , colorRange(this, QByteArrayLiteral("COLOR_RANGE"),
                 {
                         QByteArrayLiteral("YCbCr limited range"),
                         QByteArrayLiteral("YCbCr full range"),
                 })
    , vmHotspotX(this, QByteArrayLiteral("HOTSPOT_X"))
    , vmHotspotY(this, QByteArrayLiteral("HOTSPOT_Y"))
    , inFenceFd(this, QByteArrayLiteral("IN_FENCE_FD"))
{
}

bool DrmPlane::updateProperties()
{
    DrmUniquePtr<drmModePlane> p(drmModeGetPlane(device()->fd(), id()));
    if (!p) {
        qCWarning(gLcDrm) << "Failed to get kernel plane" << id();
        return false;
    }
    DrmPropertyList props = queryProperties();
    type.update(props);
    srcX.update(props);
    srcY.update(props);
    srcW.update(props);
    srcH.update(props);
    crtcX.update(props);
    crtcY.update(props);
    crtcW.update(props);
    crtcH.update(props);
    fbId.update(props);
    crtcId.update(props);
    rotation.update(props);
    inFormats.update(props);
    alpha.update(props);
    pixelBlendMode.update(props);
    colorEncoding.update(props);
    colorRange.update(props);
    vmHotspotX.update(props);
    vmHotspotY.update(props);
    inFenceFd.update(props);

    if (!type.isValid() || !srcX.isValid() || !srcY.isValid() || !srcW.isValid() || !srcH.isValid()
        || !crtcX.isValid() || !crtcY.isValid() || !crtcW.isValid() || !crtcH.isValid()
        || !fbId.isValid()) {
        return false;
    }

    m_possibleCrtcs = p->possible_crtcs;

    // read formats from blob if available and if modifiers are supported, and from the plane object
    // if not
    m_supportedFormats.clear();
    if (inFormats.isValid() && inFormats.immutableBlob() && device()->hasAddFB2ModifiersSupport()) {
        drmModeFormatModifierIterator iterator{};
        while (drmModeFormatModifierBlobIterNext(inFormats.immutableBlob(), &iterator)) {
            m_supportedFormats[iterator.fmt].push_back(iterator.mod);
        }
    } else {
        // if we don't have modifier support, assume the cursor needs a linear buffer
        const QList<uint64_t> modifiers = { type.enumValue() == TypeIndex::Cursor
                                                    ? DRM_FORMAT_MOD_LINEAR
                                                    : DRM_FORMAT_MOD_INVALID };
        for (uint32_t i = 0; i < p->count_formats; i++) {
            m_supportedFormats.insert(p->formats[i], modifiers);
        }
        if (m_supportedFormats.isEmpty()) {
            qCWarning(gLcDrm) << "Driver doesn't advertise any formats for this plane. Falling "
                                 "back to XRGB8888 without explicit modifiers";
            m_supportedFormats.insert(DRM_FORMAT_XRGB8888, modifiers);
        }
    }
    return true;
}

#if 0
void DrmPlane::set(DrmAtomicCommit *commit, const QPoint &srcPos, const QSize &srcSize,
                   const QRect &dst)
{
    // Src* are in 16.16 fixed point format
    commit->addProperty(srcX, srcPos.x() << 16);
    commit->addProperty(srcX, srcPos.x() << 16);
    commit->addProperty(srcY, srcPos.y() << 16);
    commit->addProperty(srcW, srcSize.width() << 16);
    commit->addProperty(srcH, srcSize.height() << 16);
    commit->addProperty(crtcX, dst.x());
    commit->addProperty(crtcY, dst.y());
    commit->addProperty(crtcW, dst.width());
    commit->addProperty(crtcH, dst.height());
}
#endif

bool DrmPlane::isCrtcSupported(int pipeIndex) const
{
    return (m_possibleCrtcs & (1 << pipeIndex));
}

QMap<uint32_t, QList<uint64_t>> DrmPlane::formats() const
{
    return m_supportedFormats;
}

#if 0
std::shared_ptr<DrmFramebuffer> DrmPlane::currentBuffer() const
{
    return m_current;
}

void DrmPlane::setCurrentBuffer(const std::shared_ptr<DrmFramebuffer> &b)
{
    m_current = b;
}

void DrmPlane::disable(DrmAtomicCommit *commit)
{
    commit->addProperty(crtcId, 0);
    commit->addBuffer(this, nullptr);
}

void DrmPlane::releaseCurrentBuffer()
{
    if (m_current)
        m_current->releaseBuffer();
}
#endif

int32_t DrmPlane::transformationToDegrees(DrmPlane::Transformations transformation)
{
    if (transformation & DrmPlane::Transformation::Rotate0) {
        return 0;
    } else if (transformation & DrmPlane::Transformation::Rotate90) {
        return 90;
    } else if (transformation & DrmPlane::Transformation::Rotate180) {
        return 180;
    } else {
        return 270;
    }
}

} // namespace Platform

} // namespace Aurora
