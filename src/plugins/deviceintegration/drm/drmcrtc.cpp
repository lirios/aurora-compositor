// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2016 Roman Gilg <subdiff@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "drmcrtc.h"
#include "drmbackend.h"
#include "drmdevice.h"
#include "drmoutput.h"
#include "drmpointer.h"

namespace Aurora {

namespace Platform {

DrmCrtc::DrmCrtc(DrmDevice *device, uint32_t crtcId, int pipeIndex, DrmPlane *primaryPlane,
                 DrmPlane *cursorPlane)
    : DrmObject(device, crtcId, DRM_MODE_OBJECT_CRTC)
    , modeId(this, QByteArrayLiteral("MODE_ID"))
    , active(this, QByteArrayLiteral("ACTIVE"))
    , vrrEnabled(this, QByteArrayLiteral("VRR_ENABLED"))
    , gammaLut(this, QByteArrayLiteral("GAMMA_LUT"))
    , gammaLutSize(this, QByteArrayLiteral("GAMMA_LUT_SIZE"))
    , ctm(this, QByteArrayLiteral("CTM"))
    , degammaLut(this, QByteArrayLiteral("DEGAMMA_LUT"))
    , degammaLutSize(this, QByteArrayLiteral("DEGAMMA_LUT_SIZE"))
    , m_crtc(drmModeGetCrtc(device->fd(), crtcId))
    , m_pipeIndex(pipeIndex)
    , m_primaryPlane(primaryPlane)
    , m_cursorPlane(cursorPlane)
{
}

bool DrmCrtc::updateProperties()
{
    if (!m_crtc)
        return false;

    DrmPropertyList props = queryProperties();
    modeId.update(props);
    active.update(props);
    vrrEnabled.update(props);
    gammaLut.update(props);
    gammaLutSize.update(props);
    ctm.update(props);
    degammaLut.update(props);
    degammaLutSize.update(props);

    return !device()->hasAtomicSupport() || (modeId.isValid() && active.isValid());
}

drmModeModeInfo DrmCrtc::queryCurrentMode()
{
    m_crtc.reset(drmModeGetCrtc(device()->fd(), id()));
    return m_crtc->mode;
}

int DrmCrtc::pipeIndex() const
{
    return m_pipeIndex;
}

#if 0
std::shared_ptr<DrmFramebuffer> DrmCrtc::current() const
{
    return m_currentBuffer;
}

void DrmCrtc::setCurrent(const std::shared_ptr<DrmFramebuffer> &buffer)
{
    m_currentBuffer = buffer;
}
#endif

int DrmCrtc::gammaRampSize() const
{
    if (device()->hasAtomicSupport()) {
        // limit atomic gamma ramp to 4096 to work around
        // https://gitlab.freedesktop.org/drm/intel/-/issues/3916
        if (gammaLutSize.isValid() && gammaLutSize.value() <= 4096)
            return gammaLutSize.value();
    }
    return m_crtc->gamma_size;
}

DrmPlane *DrmCrtc::primaryPlane() const
{
    return m_primaryPlane;
}

DrmPlane *DrmCrtc::cursorPlane() const
{
    return m_cursorPlane;
}

#if 0
void DrmCrtc::disable(DrmAtomicCommit *commit)
{
    commit->addProperty(active, 0);
    commit->addProperty(modeId, 0);
}

void DrmCrtc::releaseCurrentBuffer()
{
    if (m_currentBuffer)
        m_currentBuffer->releaseBuffer();
}
#endif

} // namespace Platform

} // namespace Aurora
