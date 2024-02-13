// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drmconnector.h"
#include "drmoutput.h"

namespace Aurora {

namespace Platform {

DrmOutput::DrmOutput(const std::shared_ptr<DrmConnector> &connector, QObject *parent)
    : Output(parent)
    , m_connector(connector)
{
    Capabilities capabilities = Capability::PowerState | Capability::IccProfile;

    if (connector->overscan.isValid() || connector->underscan.isValid()) {
        capabilities |= Capability::Overscan;
        // initialState.overscan = connector->overscan.isValid() ? connector->overscan.value() :
        // connector->underscanVBorder.value();
    }

    const Edid *edid = connector->edid();
    setInformation(Information{
            .name = connector->connectorName(),
            .manufacturer = edid->manufacturerString(),
            .model = connector->modelName(),
            .serialNumber = QString::fromLocal8Bit(edid->serialNumber()),
            .eisaId = QString::fromLocal8Bit(edid->eisaId()),
            .physicalSize = connector->physicalSize(),
            .edid = *edid,
            .subpixel = connector->subpixel(),
            .capabilities = capabilities,
            .panelOrientation = connector->panelOrientation.isValid()
                    ? DrmConnector::convertToTransform(connector->panelOrientation.enumValue())
                    : Transform::Normal,
            .internal = connector->isInternal(),
            .nonDesktop = connector->isNonDesktop(),
            .mstPath = connector->mstPath(),
            .maxPeakBrightness = edid->desiredMaxLuminance(),
            .maxAverageBrightness = edid->desiredMaxFrameAverageLuminance(),
            .minBrightness = edid->desiredMinLuminance(),
    });
}

DrmOutput::~DrmOutput()
{
    destroy();
}

DrmConnector *DrmOutput::connector() const
{
    return m_connector.get();
}

void DrmOutput::destroy()
{
}

} // namespace Platform

} // namespace Aurora
