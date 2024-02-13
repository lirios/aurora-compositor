// SPDX-FileCopyrightText: 2015-2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2020 Xaver Hugl <xaver.hugl@gmail.com>
// SPDX-FileCopyrightText: 2016-2017 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2016 Pelagicore AG
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>

#include "drmbackend.h"
#include "drmcrtc.h"
#include "drmconnector.h"
#include "drmdevice.h"
#include "drmloggingcategories.h"
#include "drmobject.h"
#include "drmpointer.h"
#include "drmplane.h"

#include <fcntl.h>

#include <libdrm/drm_mode.h>
#include <xf86drm.h>

#ifndef DRM_CLIENT_CAP_CURSOR_PLANE_HOTSPOT
#  define DRM_CLIENT_CAP_CURSOR_PLANE_HOTSPOT 6
#endif

namespace Aurora {

namespace Platform {

DrmDevice::DrmDevice(const QString &path, int fd, dev_t deviceId, QObject *parent)
    : QObject(parent)
    , m_path(path)
    , m_fd(fd)
    , m_deviceId(deviceId)
{
    // Determine KMS driver for quirks
    DrmUniquePtr<drmVersion> version(drmGetVersion(m_fd));
    if (version) {
        if (strstr(version->name, "i915"))
            m_drmDriver = DrmDriver::I915;
        else if (strstr(version->name, "nvidia-drm"))
            m_drmDriver = DrmDriver::NVidia;
        else if (strstr(version->name, "virtio") || strstr(version->name, "qxl")
                 || strstr(version->name, "vmwgfx") || strstr(version->name, "vboxvideo"))
            m_drmDriver = DrmDriver::VirtualMachine;
        qCDebug(gLcDrm) << "DRM Driver:" << version->name;
    } else {
        qCCritical(gLcDrm, "Failed to get DRM version for \"%s\"", qPrintable(path));
        QCoreApplication::exit(1);
        return;
    }

    // Capabilities
    uint64_t capability = 0;
    m_hasAddFB2ModifiersSupport =
            drmGetCap(m_fd, DRM_CAP_ADDFB2_MODIFIERS, &capability) == 0 && capability == 1;
    if (drmGetCap(m_fd, DRM_CAP_CURSOR_WIDTH, &capability) == 0)
        m_cursorSize.setWidth(capability);
    if (drmGetCap(m_fd, DRM_CAP_CURSOR_HEIGHT, &capability) == 0)
        m_cursorSize.setHeight(capability);
    if (drmGetCap(m_fd, DRM_CAP_TIMESTAMP_MONOTONIC, &capability) == 0 && capability == 1)
        m_presentationClock = CLOCK_MONOTONIC;
    else
        m_presentationClock = CLOCK_REALTIME;
    qCDebug(gLcDrm, "Capabilities for \"%s\":", qPrintable(path));
    qCDebug(gLcDrm) << "\tdrmModeAddFB2WithModifiers:"
                    << (m_hasAddFB2ModifiersSupport ? "supported" : "not supported");
    qCDebug(gLcDrm, "\tCursor size: %dx%d", m_cursorSize.width(), m_cursorSize.height());
    qCDebug(gLcDrm) << "\tPresentation clock:"
                    << (m_presentationClock == CLOCK_MONOTONIC ? "monotonic" : "realtime");

    // Reopen the drm node to create a new GEM handle namespace
    m_gbmFd = FileDescriptor(open(qPrintable(path), O_RDWR | O_CLOEXEC));
    if (m_gbmFd.isValid()) {
        drm_magic_t magic;
        drmGetMagic(m_gbmFd.get(), &magic);
        drmAuthMagic(m_fd, magic);

        // Create GBM device
        m_gbmDevice = gbm_create_device(m_gbmFd.get());
        if (!m_gbmDevice) {
            qCCritical(gLcDrm, "Could not create GBM device for \"%s\": %s", qPrintable(path),
                       strerror(errno));
            QCoreApplication::exit(1);
            return;
        }
    }

    // Event reader
    m_eventReader.create(this);

    // Enable universal planes
    drmSetClientCap(m_fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

    // Do we have atomic support?
    m_hasAtomicSupport = drmSetClientCap(m_fd, DRM_CLIENT_CAP_ATOMIC, 1) == 0;
    if (m_hasAtomicSupport) {
        qCInfo(gLcDrm, "Atomic reported as supported on \"%s\"", qPrintable(path));

        const bool atomicDisabled = qEnvironmentVariableIsSet("AURORA_KMS_ATOMIC")
                && !qEnvironmentVariableIntValue("AURORA_KMS_ATOMIC");
        if (atomicDisabled) {
            m_hasAtomicSupport = false;
        } else {
            // Does it support cursor plane hotspot?
            const bool supportsCursorHotspot =
                    drmSetClientCap(m_fd, DRM_CLIENT_CAP_CURSOR_PLANE_HOTSPOT, 1) == 0;
            if (m_drmDriver == DrmDriver::VirtualMachine && !supportsCursorHotspot) {
                qCWarning(gLcDrm,
                          "Atomic disabled on \"%s\" due to missing cursor offset support in "
                          "virtual machines",
                          qPrintable(path));
                m_hasAtomicSupport = false;
            }
        }
    }

    // Initialize DRM resources
    initDrmResources();
}

DrmDevice::~DrmDevice()
{
    m_eventReader.destroy();

    if (m_gbmDevice)
        gbm_device_destroy(m_gbmDevice);

    m_gbmFd.reset();

    if (m_fd != -1)
        DrmBackend::instance()->session()->closeRestricted(m_fd);
}

QString DrmDevice::deviceNode() const
{
    return m_path;
}

int DrmDevice::fd() const
{
    return m_fd;
}

dev_t DrmDevice::deviceId() const
{
    return m_deviceId;
}

gbm_device *DrmDevice::gbmDevice() const
{
    return m_gbmDevice;
}

bool DrmDevice::isActive() const
{
    return m_active;
}

void DrmDevice::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        Q_EMIT activeChanged(active);
    }
}

bool DrmDevice::isRemoved() const
{
    return m_removed;
}

void DrmDevice::setRemoved()
{
    // Once the device is removed we no longer reuse it
    m_removed = true;
}

bool DrmDevice::hasAtomicSupport() const
{
    return m_hasAddFB2ModifiersSupport;
}

bool DrmDevice::hasAddFB2ModifiersSupport() const
{
    return false;
}

bool DrmDevice::updateOutputs()
{
    if (!isActive())
        return false;

    // Get resources
    DrmUniquePtr<drmModeRes> resources(drmModeGetResources(m_fd));
    if (!resources) {
        qCWarning(gLcDrm, "Failed to get DRM resources for \"%s\": %s", qPrintable(m_path),
                  strerror(errno));
        return false;
    }

    if (resources->count_connectors == 0) {
        qCWarning(gLcDrm, "No connectors found for \"%s\"", qPrintable(m_path));
        return false;
    }

    // Check for added or removed connectors
    QList<DrmConnector *> existing;
    QList<DrmOutput *> addedOutputs;
    for (int i = 0; i < resources->count_connectors; i++) {
        const uint32_t currentConnector = resources->connectors[i];
        const auto it = std::find_if(m_connectors.begin(), m_connectors.end(),
                                     [currentConnector](const auto &connector) {
                                         return connector->id() == currentConnector;
                                     });
        if (it == m_connectors.end()) {
            auto conn = std::make_shared<DrmConnector>(this, currentConnector);
            if (!conn->init())
                continue;
            existing.push_back(conn.get());
            m_allObjects.push_back(conn.get());
            m_connectors.push_back(std::move(conn));
        } else {
            (*it)->updateProperties();
            existing.push_back(it->get());
        }
    }
    for (auto it = m_connectors.begin(); it != m_connectors.end();) {
        DrmConnector *conn = it->get();
        const auto output = findOutput(conn->id());
        const bool stillExists = existing.contains(conn);
        if (!stillExists || !conn->isConnected()) {
            if (output) {
                removeOutput(output);
            }
        } else if (!output) {
            qCDebug(gLcDrm, "New %soutput on \"%s\": %s",
                    conn->isNonDesktop() ? "non-desktop " : "", qPrintable(m_path),
                    qPrintable(conn->modelName()));
#if 0
            const auto pipeline = conn->pipeline();
            m_pipelines << pipeline;
#endif
            auto output = new DrmOutput(*it);
            m_drmOutputs << output;
            addedOutputs << output;
            Q_EMIT outputAdded(output);
#if 0
            pipeline->setLayers(m_platform->renderBackend()->createPrimaryLayer(pipeline),
                                m_platform->renderBackend()->createCursorLayer(pipeline));
            pipeline->setActive(!conn->isNonDesktop());
            pipeline->applyPendingChanges();
#endif
        }
        if (stillExists) {
            it++;
        } else {
            m_allObjects.removeOne(it->get());
            it = m_connectors.erase(it);
        }
    }

    // Update crtc properties
    for (const auto &crtc : std::as_const(m_crtcs)) {
        crtc->updateProperties();
    }

    // Update plane properties
    for (const auto &plane : std::as_const(m_planes)) {
        plane->updateProperties();
    }

#if 0
    DrmPipeline::Error err = testPendingConfiguration();
    if (err == DrmPipeline::Error::None) {
        for (const auto &pipeline : std::as_const(m_pipelines)) {
            pipeline->applyPendingChanges();
            if (pipeline->output() && !pipeline->crtc()) {
                pipeline->setEnable(false);
                pipeline->output()->updateEnabled(false);
            }
        }
    } else if (err == DrmPipeline::Error::NoPermission) {
        for (const auto &pipeline : std::as_const(m_pipelines)) {
            pipeline->revertPendingChanges();
        }
        for (const auto &output : std::as_const(addedOutputs)) {
            removeOutput(output);
            const auto it = std::find_if(
                    m_connectors.begin(), m_connectors.end(),
                    [output](const auto &conn) { return conn.get() == output->connector(); });
            Q_ASSERT(it != m_connectors.end());
            m_allObjects.removeOne(it->get());
            m_connectors.erase(it);
        }
    } else {
        qCWarning(KWIN_DRM, "Failed to find a working setup for new outputs!");
        for (const auto &pipeline : std::as_const(m_pipelines)) {
            pipeline->revertPendingChanges();
        }
        for (const auto &output : std::as_const(addedOutputs)) {
            output->updateEnabled(false);
            output->pipeline()->setEnable(false);
            output->pipeline()->applyPendingChanges();
        }
    }
#endif

    return true;
}

void DrmDevice::removeOutputs()
{
}

void DrmDevice::removeOutput(DrmOutput *output)
{
    qCDebug(gLcDrm) << "Removing output" << output;
    // m_pipelines.removeOne(output->pipeline());
    // output->pipeline()->setLayers(nullptr, nullptr);
    m_drmOutputs.removeOne(output);
    Q_EMIT outputRemoved(output);
    // output->unref();
}

QList<DrmOutput *> DrmDevice::drmOutputs() const
{
    return m_drmOutputs;
}

DrmOutput *DrmDevice::findOutput(quint32 connectorId)
{
    auto it = std::find_if(
            m_drmOutputs.constBegin(), m_drmOutputs.constEnd(),
            [connectorId](DrmOutput *o) { return o->connector()->id() == connectorId; });
    if (it != m_drmOutputs.constEnd()) {
        return *it;
    }
    return nullptr;
}

void DrmDevice::initDrmResources()
{
    // Find planes
    DrmUniquePtr<drmModePlaneRes> planeResources(drmModeGetPlaneResources(m_fd));
    if (planeResources) {
        qCDebug(gLcDrm, "Number of planes on \"%s\": %d", qPrintable(m_path),
                planeResources->count_planes);
        for (unsigned int i = 0; i < planeResources->count_planes; ++i) {
            DrmUniquePtr<drmModePlane> drmPlane(drmModeGetPlane(m_fd, planeResources->planes[i]));
            auto plane = std::make_unique<DrmPlane>(this, drmPlane->plane_id);
            if (plane->init()) {
                m_allObjects.append(plane.get());
                m_planes.push_back(std::move(plane));
            }
        }
    } else {
        qCWarning(gLcDrm, "Failed to get planes for \"%s\", disabling atomic", qPrintable(m_path));
        m_hasAtomicSupport = false;
    }

    // Show wheather atmoc is supported (at this point support might be disabled
    // during planes discovery)
    if (m_hasAtomicSupport)
        qCInfo(gLcDrm, "Atomic enabled on \"%s\"", qPrintable(m_path));
    else
        qCInfo(gLcDrm, "Atomic disabled on \"%s\"", qPrintable(m_path));

    // Get resources
    DrmUniquePtr<drmModeRes> resources(drmModeGetResources(m_fd));
    if (!resources) {
        qCCritical(gLcDrm, "Failed to get DRM resources for \"%s\": %s", qPrintable(m_path),
                   strerror(errno));
        QCoreApplication::exit(1);
        return;
    }

    if (resources->count_connectors == 0) {
        qCCritical(gLcDrm, "No connectors found for \"%s\"", qPrintable(m_path));
        QCoreApplication::exit(1);
        return;
    }

    // Find best plane
    QList<DrmPlane *> assignedPlanes;
    for (int i = 0; i < resources->count_crtcs; ++i) {
        uint32_t crtcId = resources->crtcs[i];

        QList<DrmPlane *> primaryCandidates;
        QList<DrmPlane *> cursorCandidates;

        for (const auto &plane : m_planes) {
            if (plane->isCrtcSupported(i) && !assignedPlanes.contains(plane.get())) {
                if (plane->type.enumValue() == DrmPlane::TypeIndex::Primary)
                    primaryCandidates.push_back(plane.get());
                else if (plane->type.enumValue() == DrmPlane::TypeIndex::Cursor)
                    cursorCandidates.push_back(plane.get());
            }
        }

        if (m_hasAtomicSupport && primaryCandidates.empty()) {
            qCWarning(gLcDrm) << "Could not find a suitable primary plane for crtc"
                              << resources->crtcs[i];
            continue;
        }

        const auto findBestPlane = [crtcId](const QList<DrmPlane *> &list) {
            // if the plane is already used with this crtc, prefer it
            const auto connected =
                    std::find_if(list.begin(), list.end(), [crtcId](DrmPlane *plane) {
                        return plane->crtcId.value() == crtcId;
                    });
            if (connected != list.end()) {
                return *connected;
            }
            // don't take away planes from other crtcs. The kernel currently rejects such commits
            const auto notconnected = std::find_if(list.begin(), list.end(), [](DrmPlane *plane) {
                return plane->crtcId.value() == 0;
            });
            if (notconnected != list.end())
                return *notconnected;
            return list.empty() ? nullptr : list.front();
        };

        DrmPlane *primary = findBestPlane(primaryCandidates);
        DrmPlane *cursor = findBestPlane(cursorCandidates);
        assignedPlanes.push_back(primary);
        if (cursor)
            assignedPlanes.push_back(cursor);
        auto crtc = std::make_unique<DrmCrtc>(this, crtcId, i, primary, cursor);
        if (!crtc->init())
            continue;
        m_allObjects.append(crtc.get());
        m_crtcs.push_back(std::move(crtc));
    }
}

} // namespace Platform

} // namespace Aurora
