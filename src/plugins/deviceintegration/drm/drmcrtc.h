// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2016 Roman Gilg <subdiff@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QPoint>

#include "drmobject.h"

#include <memory>

namespace Aurora {

namespace Platform {

class DrmBackend;
class DrmDevice;
#if 0
class DrmFramebuffer;
#endif
class GammaRamp;
class DrmPlane;

class DrmCrtc : public DrmObject
{
public:
    DrmCrtc(DrmDevice *device, uint32_t crtcId, int pipeIndex, DrmPlane *primaryPlane,
            DrmPlane *cursorPlane);

#if 0
    void disable(DrmAtomicCommit *commit) override;
#endif
    bool updateProperties() override;

    int pipeIndex() const;
    int gammaRampSize() const;
    DrmPlane *primaryPlane() const;
    DrmPlane *cursorPlane() const;
    drmModeModeInfo queryCurrentMode();

#if 0
    std::shared_ptr<DrmFramebuffer> current() const;
    void setCurrent(const std::shared_ptr<DrmFramebuffer> &buffer);
    void releaseCurrentBuffer();
#endif

    DrmProperty modeId;
    DrmProperty active;
    DrmProperty vrrEnabled;
    DrmProperty gammaLut;
    DrmProperty gammaLutSize;
    DrmProperty ctm;
    DrmProperty degammaLut;
    DrmProperty degammaLutSize;

private:
    DrmUniquePtr<drmModeCrtc> m_crtc;
    #if 0
    std::shared_ptr<DrmFramebuffer> m_currentBuffer;
    #endif
    int m_pipeIndex;
    DrmPlane *m_primaryPlane;
    DrmPlane *m_cursorPlane;
};

} // namespace Platform

} // namespace Aurora
