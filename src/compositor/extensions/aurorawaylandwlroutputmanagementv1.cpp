// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aurorawaylandclient.h"
#include "aurorawaylandwlroutputmanagementv1_p.h"

#include <wayland-server-protocol.h>

namespace Aurora {

namespace Compositor {

Q_LOGGING_CATEGORY(gLcWaylandWlrOutputManagementV1, "aurora.compositor.wlroutputmanagementv1")

/*
 * WaylandWlrOutputManagerV1Private
 */

WaylandWlrOutputManagerV1Private::WaylandWlrOutputManagerV1Private()
{
}

WaylandWlrOutputManagerV1Private::~WaylandWlrOutputManagerV1Private()
{
    const auto values = resourceMap().values();
    for (auto *resource : values)
        send_finished(resource->handle);
}

void WaylandWlrOutputManagerV1Private::registerHead(WaylandWlrOutputHeadV1 *head)
{
    Q_Q(WaylandWlrOutputManagerV1);

    if (heads.contains(head))
        return;

    heads.append(head);

    Q_EMIT q->headAdded(head);
}

void WaylandWlrOutputManagerV1Private::zwlr_output_manager_v1_bind_resource(Resource *resource)
{
    if (!compositor)
        return;

    // Send all the heads at once when the client binds
    for (auto *head : qAsConst(heads)) {
        auto *headPrivate = WaylandWlrOutputHeadV1Private::get(head);
        auto *headResource = headPrivate->add(resource->client(), headPrivate->interfaceVersion());
        send_head(resource->handle, headResource->handle);
        headPrivate->sendInfo(headResource);
    }
    if (heads.count() > 0)
        send_done(resource->handle, compositor->nextSerial());
}

void WaylandWlrOutputManagerV1Private::zwlr_output_manager_v1_create_configuration(Resource *resource, uint32_t id, uint32_t serial)
{
    Q_Q(WaylandWlrOutputManagerV1);

    if (stoppedClients.contains(resource->client()))
        return;

    WaylandResource configResource(wl_resource_create(
                                       resource->client(), &zwlr_output_configuration_v1_interface,
                                       wl_resource_get_version(resource->handle), id));

    emit q->configurationRequested(configResource);

    auto *config = WaylandWlrOutputConfigurationV1::fromResource(configResource.resource());
    if (!config) {
        // WaylandWlrOutputConfigurationV1 was not created in response to the configurationRequested signal
        // so we have to create a fallback one here
        config = new WaylandWlrOutputConfigurationV1(q, configResource);
    }

    configurations[serial] = config;

    emit q->configurationCreated(config);
}

void WaylandWlrOutputManagerV1Private::zwlr_output_manager_v1_stop(Resource *resource)
{
    Q_Q(WaylandWlrOutputManagerV1);

    stoppedClients.append(resource->client());

    Q_EMIT q->clientStopped(WaylandClient::fromWlClient(compositor, resource->client()));
}

/*
 * WaylandWlrOutputManagerV1
 */

WaylandWlrOutputManagerV1::WaylandWlrOutputManagerV1()
    : WaylandCompositorExtensionTemplate<WaylandWlrOutputManagerV1>(*new WaylandWlrOutputManagerV1Private)
{
}

WaylandWlrOutputManagerV1::WaylandWlrOutputManagerV1(WaylandCompositor *compositor)
    : WaylandCompositorExtensionTemplate<WaylandWlrOutputManagerV1>(compositor, *new WaylandWlrOutputManagerV1Private)
{
    Q_D(WaylandWlrOutputManagerV1);
    d->compositor = compositor;
}

WaylandWlrOutputManagerV1::~WaylandWlrOutputManagerV1()
{
}

void WaylandWlrOutputManagerV1::initialize()
{
    Q_D(WaylandWlrOutputManagerV1);

    WaylandCompositorExtensionTemplate::initialize();
    WaylandCompositor *compositor = static_cast<WaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qCWarning(gLcWaylandWlrOutputManagementV1, "Failed to find WaylandCompositor when initializing WlrOutputManagerV1");
        return;
    }
    d->compositor = compositor;
    d->init(compositor->display(), WaylandWlrOutputManagerV1Private::interfaceVersion());

    d->initialized = true;
}

WaylandCompositor *WaylandWlrOutputManagerV1::compositor() const
{
    Q_D(const WaylandWlrOutputManagerV1);
    return d->compositor;
}

void WaylandWlrOutputManagerV1::setCompositor(WaylandCompositor *compositor)
{
    Q_D(WaylandWlrOutputManagerV1);

    if (d->compositor == compositor)
        return;

    if (d->initialized) {
        qCWarning(gLcWaylandWlrOutputManagementV1, "Can't set WaylandCompositor after WlrOutputManagerV1 initialization");
        return;
    }

    d->compositor = compositor;
    Q_EMIT compositorChanged();
}

QVector<WaylandWlrOutputHeadV1 *> WaylandWlrOutputManagerV1::heads() const
{
    Q_D(const WaylandWlrOutputManagerV1);
    return d->heads;
}

void WaylandWlrOutputManagerV1::done(quint32 serial)
{
    Q_D(WaylandWlrOutputManagerV1);

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_done(resource->handle, serial);
}

void WaylandWlrOutputManagerV1::finished()
{
    Q_D(WaylandWlrOutputManagerV1);

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_finished(resource->handle);
}

const wl_interface *WaylandWlrOutputManagerV1::interface()
{
    return WaylandWlrOutputManagerV1Private::interface();
}

QByteArray WaylandWlrOutputManagerV1::interfaceName()
{
    return WaylandWlrOutputManagerV1Private::interfaceName();
}

/*
 * WaylandWlrOutputHeadV1Private
 */

WaylandWlrOutputHeadV1Private::WaylandWlrOutputHeadV1Private()
{
}

WaylandWlrOutputHeadV1Private::~WaylandWlrOutputHeadV1Private()
{
    const auto values = resourceMap().values();
    for (auto *resource : values)
        send_finished(resource->handle);

    qDeleteAll(modes);
    modes.clear();
}

void WaylandWlrOutputHeadV1Private::sendInfo(Resource *resource)
{
    modesSent = true;

    send_name(resource->handle, name);
    send_description(resource->handle, description);
    if (physicalSize.width() > 0 && physicalSize.height() > 0)
        send_physical_size(resource->handle, physicalSize.width(), physicalSize.height());
    send_enabled(resource->handle, enabled ? 1 : 0);
    if (enabled) {
        send_position(resource->handle, position.x(), position.y());
        send_transform(resource->handle, static_cast<int32_t>(transform));
        send_scale(resource->handle, wl_fixed_from_double(scale));
    }

    for (auto *mode : qAsConst(modes)) {
        auto *modePrivate = WaylandWlrOutputModeV1Private::get(mode);
        auto *modeResource = modePrivate->add(resource->client(), modePrivate->interfaceVersion());
        send_mode(resource->handle, modeResource->handle);
        modePrivate->send_size(modeResource->handle, modePrivate->size.width(), modePrivate->size.height());
        modePrivate->send_refresh(modeResource->handle, modePrivate->refreshRate);

        if (enabled && currentMode) {
            if (mode == currentMode)
                send_current_mode(resource->handle, modeResource->handle);
        }

        if (enabled && preferredMode) {
            if (mode == preferredMode)
                modePrivate->send_preferred(modeResource->handle);
        }
    }
}

WaylandWlrOutputHeadV1 *WaylandWlrOutputHeadV1Private::fromResource(wl_resource *resource)
{
    return static_cast<WaylandWlrOutputHeadV1Private *>(WaylandWlrOutputHeadV1Private::Resource::fromResource(resource)->zwlr_output_head_v1_object)->q_func();
}


WaylandWlrOutputHeadV1::WaylandWlrOutputHeadV1(QObject *parent)
    : QObject(*new WaylandWlrOutputHeadV1Private, parent)
{
}

WaylandWlrOutputHeadV1::~WaylandWlrOutputHeadV1()
{
    Q_D(WaylandWlrOutputHeadV1);

    if (d->manager)
        WaylandWlrOutputManagerV1Private::get(d->manager)->heads.removeOne(this);
}

bool WaylandWlrOutputHeadV1::isInitialized() const
{
    Q_D(const WaylandWlrOutputHeadV1);
    return d->initialized;
}

void WaylandWlrOutputHeadV1::initialize()
{
    Q_D(WaylandWlrOutputHeadV1);

    if (d->manager) {
        WaylandWlrOutputManagerV1Private::get(d->manager)->registerHead(this);
        d->initialized = true;
    }
}

WaylandWlrOutputManagerV1 *WaylandWlrOutputHeadV1::manager() const
{
    Q_D(const WaylandWlrOutputHeadV1);
    return d->manager;
}

void WaylandWlrOutputHeadV1::setManager(WaylandWlrOutputManagerV1 *manager)
{
    Q_D(WaylandWlrOutputHeadV1);

    if (d->manager) {
        qCWarning(gLcWaylandWlrOutputManagementV1, "Output manager already set, cannot override");
        return;
    }

    d->manager = manager;
    Q_EMIT managerChanged();

    initialize();
}

bool WaylandWlrOutputHeadV1::isEnabled() const
{
    Q_D(const WaylandWlrOutputHeadV1);
    return d->enabled;
}

void WaylandWlrOutputHeadV1::setEnabled(bool enabled)
{
    Q_D(WaylandWlrOutputHeadV1);

    if (d->enabled == enabled)
        return;

    d->enabled = enabled;

    if (d->initialized) {
        const auto values = d->resourceMap().values();
        for (auto *resource : values)
            d->send_enabled(resource->handle, enabled ? 1 : 0);
        manager()->done(manager()->compositor()->nextSerial());
    }

    Q_EMIT enabledChanged();
}

QString WaylandWlrOutputHeadV1::name() const
{
    Q_D(const WaylandWlrOutputHeadV1);
    return d->name;
}

void WaylandWlrOutputHeadV1::setName(const QString &name)
{
    Q_D(WaylandWlrOutputHeadV1);

    if (d->name == name)
        return;

    // Can only be changed the first time
    if (d->nameChanged) {
        qCWarning(gLcWaylandWlrOutputManagementV1, "The name does not change over the lifetime of the output head");
        return;
    }

    d->name = name;
    d->nameChanged = true;

    Q_EMIT nameChanged();
}

QString WaylandWlrOutputHeadV1::description() const
{
    Q_D(const WaylandWlrOutputHeadV1);
    return d->name;
}

void WaylandWlrOutputHeadV1::setDescription(const QString &description)
{
    Q_D(WaylandWlrOutputHeadV1);

    if (d->description == description)
        return;

    // Can only be changed the first time
    if (d->descriptionChanged) {
        qCWarning(gLcWaylandWlrOutputManagementV1, "The description does not change over the lifetime of the output head");
        return;
    }

    d->description = description;
    d->descriptionChanged = true;

    Q_EMIT descriptionChanged();
}

QSize WaylandWlrOutputHeadV1::physicalSize() const
{
    Q_D(const WaylandWlrOutputHeadV1);
    return d->physicalSize;
}

void WaylandWlrOutputHeadV1::setPhysicalSize(const QSize &physicalSize)
{
    Q_D(WaylandWlrOutputHeadV1);

    if (d->physicalSize == physicalSize)
        return;

    d->physicalSize = physicalSize;

    if (d->initialized) {
        const auto values = d->resourceMap().values();
        for (auto *resource : values)
            d->send_physical_size(resource->handle, physicalSize.width(), physicalSize.height());
        manager()->done(manager()->compositor()->nextSerial());
    }

    Q_EMIT physicalSizeChanged();
}

QPoint WaylandWlrOutputHeadV1::position() const
{
    Q_D(const WaylandWlrOutputHeadV1);
    return d->position;
}

void WaylandWlrOutputHeadV1::setPosition(const QPoint &position)
{
    Q_D(WaylandWlrOutputHeadV1);

    if (!d->enabled) {
        qCWarning(gLcWaylandWlrOutputManagementV1, "Cannot change position on a disabled output head");
        return;
    }

    if (d->position == position)
        return;

    d->position = position;

    if (d->initialized) {
        const auto values = d->resourceMap().values();
        for (auto *resource : values)
            d->send_position(resource->handle, position.x(), position.y());
        manager()->done(manager()->compositor()->nextSerial());
    }

    Q_EMIT positionChanged();
}

QVector<WaylandWlrOutputModeV1 *> WaylandWlrOutputHeadV1::modes() const
{
    Q_D(const WaylandWlrOutputHeadV1);
    return d->modes;
}

void WaylandWlrOutputHeadV1::addMode(WaylandWlrOutputModeV1 *mode)
{
    Q_D(WaylandWlrOutputHeadV1);

    if (d->modesSent) {
        qCWarning(gLcWaylandWlrOutputManagementV1, "Cannot add new modes after initialization");
        return;
    }

    d->modes.append(mode);

    Q_EMIT modeAdded(mode);
    Q_EMIT modesChanged();
}

WaylandWlrOutputModeV1 *WaylandWlrOutputHeadV1::currentMode() const
{
    Q_D(const WaylandWlrOutputHeadV1);
    return d->currentMode;
}

void WaylandWlrOutputHeadV1::setCurrentMode(WaylandWlrOutputModeV1 *mode)
{
    Q_D(WaylandWlrOutputHeadV1);

    if (!d->modes.contains(mode)) {
        qCWarning(gLcWaylandWlrOutputManagementV1, "Failed to set current mode: add modes first");
        return;
    }

    if (d->currentMode == mode)
        return;

    d->currentMode = mode;

    if (d->initialized) {
        const auto values = d->resourceMap().values();
        for (auto *resource : values)
            d->send_current_mode(resource->handle, WaylandWlrOutputModeV1Private::get(mode)->resource()->handle);
        manager()->done(manager()->compositor()->nextSerial());
    }

    Q_EMIT currentModeChanged();
}

WaylandWlrOutputModeV1 *WaylandWlrOutputHeadV1::preferredMode() const
{
    Q_D(const WaylandWlrOutputHeadV1);
    return d->preferredMode;
}

void WaylandWlrOutputHeadV1::setPreferredMode(WaylandWlrOutputModeV1 *mode)
{
    Q_D(WaylandWlrOutputHeadV1);

    if (!d->modes.contains(mode)) {
        qCWarning(gLcWaylandWlrOutputManagementV1, "Failed to set preferred mode: add modes first");
        return;
    }

    if (d->preferredMode == mode)
        return;

    d->preferredMode = mode;

    if (d->initialized) {
        auto *modePrivate = WaylandWlrOutputModeV1Private::get(mode);
        const auto values = modePrivate->resourceMap().values();
        for (auto *resource : values)
            modePrivate->send_preferred(resource->handle);
    }

    Q_EMIT preferredModeChanged();
}

WaylandOutput::Transform WaylandWlrOutputHeadV1::transform() const
{
    Q_D(const WaylandWlrOutputHeadV1);
    return d->transform;
}

void WaylandWlrOutputHeadV1::setTransform(WaylandOutput::Transform transform)
{
    Q_D(WaylandWlrOutputHeadV1);

    if (!d->enabled) {
        qCWarning(gLcWaylandWlrOutputManagementV1, "Cannot change transform on a disabled output head");
        return;
    }

    if (d->transform == transform)
        return;

    d->transform = transform;

    if (d->initialized) {
        const auto values = d->resourceMap().values();
        for (auto *resource : values)
            d->send_transform(resource->handle, static_cast<int32_t>(transform));
        manager()->done(manager()->compositor()->nextSerial());
    }

    Q_EMIT transformChanged();
}

qreal WaylandWlrOutputHeadV1::scale() const
{
    Q_D(const WaylandWlrOutputHeadV1);
    return d->scale;
}

void WaylandWlrOutputHeadV1::setScale(qreal scale)
{
    Q_D(WaylandWlrOutputHeadV1);

    if (!d->enabled) {
        qCWarning(gLcWaylandWlrOutputManagementV1, "Cannot change scale factor on a disabled output head");
        return;
    }

    if (d->scale == scale)
        return;

    d->scale = scale;

    if (d->initialized) {
        const auto values = d->resourceMap().values();
        for (auto *resource : values)
            d->send_scale(resource->handle, wl_fixed_from_double(scale));
        manager()->done(manager()->compositor()->nextSerial());
    }

    Q_EMIT scaleChanged();
}

/*
 * WaylandWlrOutputModeV1Private
 */

WaylandWlrOutputModeV1Private::WaylandWlrOutputModeV1Private()
{
}

WaylandWlrOutputModeV1Private::~WaylandWlrOutputModeV1Private()
{
    const auto values = resourceMap().values();
    for (auto *resource : values)
        send_finished(resource->handle);
}

WaylandWlrOutputModeV1 *WaylandWlrOutputModeV1Private::fromResource(wl_resource *resource)
{
    return static_cast<WaylandWlrOutputModeV1Private *>(WaylandWlrOutputModeV1Private::Resource::fromResource(resource)->zwlr_output_mode_v1_object)->q_func();
}


WaylandWlrOutputModeV1::WaylandWlrOutputModeV1(QObject *parent)
    : QObject(*new WaylandWlrOutputModeV1Private, parent)
{
}

WaylandWlrOutputModeV1::~WaylandWlrOutputModeV1()
{
}

bool WaylandWlrOutputModeV1::isInitialized() const
{
    Q_D(const WaylandWlrOutputModeV1);
    return d->initialized;
}

QSize WaylandWlrOutputModeV1::size() const
{
    Q_D(const WaylandWlrOutputModeV1);
    return d->size;
}

void WaylandWlrOutputModeV1::setSize(const QSize &size)
{
    Q_D(WaylandWlrOutputModeV1);

    if (d->size == size)
        return;

    d->size = size;

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_size(resource->handle, size.width(), size.height());

    Q_EMIT sizeChanged();
}

qint32 WaylandWlrOutputModeV1::refresh() const
{
    Q_D(const WaylandWlrOutputModeV1);
    return d->refreshRate;
}

void WaylandWlrOutputModeV1::setRefresh(qint32 refreshRate)
{
    Q_D(WaylandWlrOutputModeV1);

    if (d->refreshRate == refreshRate)
        return;

    d->refreshRate = refreshRate;

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_refresh(resource->handle, refreshRate);

    Q_EMIT refreshChanged();
}

/*
 * WaylandWlrOutputConfigurationHeadV1Private
 */

WaylandWlrOutputConfigurationHeadV1Private::WaylandWlrOutputConfigurationHeadV1Private()
{
}

void WaylandWlrOutputConfigurationHeadV1Private::zwlr_output_configuration_head_v1_set_mode(Resource *resource, wl_resource *modeResource)
{
    Q_Q(WaylandWlrOutputConfigurationHeadV1);

    if (modeChanged) {
        wl_resource_post_error(resource->handle, error_already_set, "mode already set");
        return;
    }

    auto *outputMode = WaylandWlrOutputModeV1Private::fromResource(modeResource);

    // Mode can be nullptr if the head doesn't support modes, in which
    // case we'll expose a virtual mode based on current output size
    const auto modes = WaylandWlrOutputHeadV1Private::get(head)->modes;
    const bool found = (outputMode == nullptr && modes.size() == 0) || modes.contains(outputMode);
    if (!found) {
        wl_resource_post_error(resource->handle, error_invalid_mode,
                               "mode doesn't belong to head");
        return;
    }

    mode = outputMode;
    modeChanged = true;
    Q_EMIT q->modeChanged(mode);

    if (mode) {
        customModeSize = QSize(0, 0);
        customModeRefresh = 0;
        Q_EMIT q->customModeChanged(customModeSize, customModeRefresh);
    }
}

void WaylandWlrOutputConfigurationHeadV1Private::zwlr_output_configuration_head_v1_set_custom_mode(Resource *resource, int32_t width, int32_t height, int32_t refresh)
{
    Q_UNUSED(resource)
    Q_Q(WaylandWlrOutputConfigurationHeadV1);

    if (width <= 0 || height <= 0 || refresh <= 0) {
        wl_resource_post_error(resource->handle, error_invalid_custom_mode,
                               "invalid custom mode");
        return;
    }

    if (customModeChanged) {
        wl_resource_post_error(resource->handle, error_already_set, "custom mode already set");
        return;
    }

    mode = nullptr;
    Q_EMIT q->modeChanged(mode);

    customModeSize = QSize(width, height);
    customModeRefresh = refresh;
    customModeChanged = true;
    Q_EMIT q->customModeChanged(QSize(width, height), refresh);
}

void WaylandWlrOutputConfigurationHeadV1Private::zwlr_output_configuration_head_v1_set_position(Resource *resource, int32_t x, int32_t y)
{
    Q_UNUSED(resource)
    Q_Q(WaylandWlrOutputConfigurationHeadV1);

    if (positionChanged) {
        wl_resource_post_error(resource->handle, error_already_set, "position already set");
        return;
    }

    position = QPoint(x, y);
    positionChanged = true;
    Q_EMIT q->positionChanged(position);
}

void WaylandWlrOutputConfigurationHeadV1Private::zwlr_output_configuration_head_v1_set_transform(Resource *resource, int32_t wlTransform)
{
    Q_Q(WaylandWlrOutputConfigurationHeadV1);

    if (wlTransform < WaylandOutput::TransformNormal ||
            wlTransform > WaylandOutput::TransformFlipped270) {
        wl_resource_post_error(resource->handle, error_invalid_transform,
                               "invalid transform");
        return;
    }

    if (transformChanged) {
        wl_resource_post_error(resource->handle, error_already_set, "transform already set");
        return;
    }

    transform = static_cast<WaylandOutput::Transform>(wlTransform);
    transformChanged = true;
    Q_EMIT q->transformChanged(transform);
}

void WaylandWlrOutputConfigurationHeadV1Private::zwlr_output_configuration_head_v1_set_scale(Resource *resource, wl_fixed_t scaleFixed)
{
    Q_Q(WaylandWlrOutputConfigurationHeadV1);

    qreal value = wl_fixed_to_double(scaleFixed);
    if (value <= 0) {
        wl_resource_post_error(resource->handle, error_invalid_scale,
                               "invalid scale");
        return;
    }

    if (scaleChanged) {
        wl_resource_post_error(resource->handle, error_already_set, "scale already set");
        return;
    }

    scale = value;
    scaleChanged = true;
    Q_EMIT q->scaleChanged(scale);
}


WaylandWlrOutputConfigurationHeadV1::WaylandWlrOutputConfigurationHeadV1(WaylandWlrOutputHeadV1 *head, QObject *parent)
    : QObject(*new WaylandWlrOutputConfigurationHeadV1Private, parent)
{
    Q_D(WaylandWlrOutputConfigurationHeadV1);
    d->head = head;
}

WaylandWlrOutputConfigurationHeadV1::~WaylandWlrOutputConfigurationHeadV1()
{
}

WaylandWlrOutputHeadV1 *WaylandWlrOutputConfigurationHeadV1::head() const
{
    Q_D(const WaylandWlrOutputConfigurationHeadV1);
    return d->head;
}

WaylandWlrOutputModeV1 *WaylandWlrOutputConfigurationHeadV1::mode() const
{
    Q_D(const WaylandWlrOutputConfigurationHeadV1);
    return d->mode;
}

QSize WaylandWlrOutputConfigurationHeadV1::customModeSize() const
{
    Q_D(const WaylandWlrOutputConfigurationHeadV1);
    return d->customModeSize;
}

qint32 WaylandWlrOutputConfigurationHeadV1::customModeRefresh() const
{
    Q_D(const WaylandWlrOutputConfigurationHeadV1);
    return d->customModeRefresh;
}

QPoint WaylandWlrOutputConfigurationHeadV1::position() const
{
    Q_D(const WaylandWlrOutputConfigurationHeadV1);
    return d->position;
}

WaylandOutput::Transform WaylandWlrOutputConfigurationHeadV1::transform() const
{
    Q_D(const WaylandWlrOutputConfigurationHeadV1);
    return d->transform;
}

qreal WaylandWlrOutputConfigurationHeadV1::scale() const
{
    Q_D(const WaylandWlrOutputConfigurationHeadV1);
    return d->scale;
}

/*
 * WaylandWlrOutputConfigurationV1Private
 */

WaylandWlrOutputConfigurationV1Private::WaylandWlrOutputConfigurationV1Private()
    : PrivateServer::zwlr_output_configuration_v1()
{
}

void WaylandWlrOutputConfigurationV1Private::zwlr_output_configuration_v1_enable_head(Resource *resource, uint32_t id, wl_resource *headResource)
{
    Q_Q(WaylandWlrOutputConfigurationV1);

    auto *head = WaylandWlrOutputHeadV1Private::fromResource(headResource);
    if (!head) {
        qCWarning(gLcWaylandWlrOutputManagementV1, "Invalid head resource");
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_NO_MEMORY,
                               "invalid head resource");
        return;
    }

    if (configuredHeads.contains(head)) {
        wl_resource_post_error(resource->handle, error_already_configured_head,
                               "already configured head");
        return;
    }

    auto version = WaylandWlrOutputConfigurationHeadV1Private::interfaceVersion();
    auto *changes = new WaylandWlrOutputConfigurationHeadV1(head, q);
    WaylandWlrOutputConfigurationHeadV1Private::get(changes)->init(resource->client(), id, version);

    configuredHeads.append(head);
    enabledHeads.append(changes);

    Q_EMIT q->headEnabled(changes);
    Q_EMIT q->enabledHeadsChanged();
}

void WaylandWlrOutputConfigurationV1Private::zwlr_output_configuration_v1_disable_head(Resource *resource, wl_resource *headResource)
{
    Q_UNUSED(resource)
    Q_Q(WaylandWlrOutputConfigurationV1);

    auto *head = WaylandWlrOutputHeadV1Private::fromResource(headResource);
    if (!head) {
        qCWarning(gLcWaylandWlrOutputManagementV1, "Invalid head resource");
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_NO_MEMORY,
                               "invalid head resource");
        return;
    }

    if (configuredHeads.contains(head)) {
        wl_resource_post_error(resource->handle, error_already_configured_head,
                               "already configured head");
        return;
    }

    configuredHeads.append(head);
    disabledHeads.append(head);

    Q_EMIT q->headDisabled(head);
    Q_EMIT q->disabledHeadsChanged();
}

void WaylandWlrOutputConfigurationV1Private::zwlr_output_configuration_v1_apply(Resource *resource)
{
    Q_Q(WaylandWlrOutputConfigurationV1);

    const auto values = manager->heads();
    for (auto *head : values) {
        if (!configuredHeads.contains(head)) {
            wl_resource_post_error(resource->handle, error_unconfigured_head,
                                   "unconfigured head %s", qPrintable(head->name()));
            return;
        }
    }

    Q_EMIT q->readyToApply();
}

void WaylandWlrOutputConfigurationV1Private::zwlr_output_configuration_v1_test(Resource *resource)
{
    Q_Q(WaylandWlrOutputConfigurationV1);

    const auto values = manager->heads();
    for (auto *head : values) {
        if (!configuredHeads.contains(head)) {
            wl_resource_post_error(resource->handle, error_unconfigured_head,
                                   "unconfigured head %s", qPrintable(head->name()));
            return;
        }
    }

    Q_EMIT q->readyToTest();
}

void WaylandWlrOutputConfigurationV1Private::zwlr_output_configuration_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

/*
 * WaylandWlrOutputConfigurationV1
 */

WaylandWlrOutputConfigurationV1::WaylandWlrOutputConfigurationV1(QObject *parent)
    : QObject(*new WaylandWlrOutputConfigurationV1Private, parent)
{
}

WaylandWlrOutputConfigurationV1::WaylandWlrOutputConfigurationV1(WaylandWlrOutputManagerV1 *manager, const WaylandResource &resource)
    : QObject(*new WaylandWlrOutputConfigurationV1Private, manager)
{
    initialize(manager, resource);
}

WaylandWlrOutputConfigurationV1::~WaylandWlrOutputConfigurationV1()
{
}

QVector<WaylandWlrOutputConfigurationHeadV1 *> WaylandWlrOutputConfigurationV1::enabledHeads() const
{
    Q_D(const WaylandWlrOutputConfigurationV1);
    return d->enabledHeads;
}

QVector<WaylandWlrOutputHeadV1 *> WaylandWlrOutputConfigurationV1::disabledHeads() const
{
    Q_D(const WaylandWlrOutputConfigurationV1);
    return d->disabledHeads;
}

void WaylandWlrOutputConfigurationV1::initialize(WaylandWlrOutputManagerV1 *manager, const WaylandResource &resource)
{
    Q_D(WaylandWlrOutputConfigurationV1);
    d->manager = manager;
    d->init(resource.resource());
}

void WaylandWlrOutputConfigurationV1::sendSucceeded()
{
    Q_D(WaylandWlrOutputConfigurationV1);

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_succeeded(resource->handle);
}

void WaylandWlrOutputConfigurationV1::sendFailed()
{
    Q_D(WaylandWlrOutputConfigurationV1);

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_failed(resource->handle);
}

void WaylandWlrOutputConfigurationV1::sendCancelled()
{
    Q_D(WaylandWlrOutputConfigurationV1);

    const auto values = d->resourceMap().values();
    for (auto *resource : values)
        d->send_cancelled(resource->handle);
}

WaylandWlrOutputConfigurationV1 *WaylandWlrOutputConfigurationV1::fromResource(struct ::wl_resource *resource)
{
    WaylandWlrOutputConfigurationV1Private::Resource *res = WaylandWlrOutputConfigurationV1Private::Resource::fromResource(resource);
    if (res)
        return static_cast<WaylandWlrOutputConfigurationV1Private *>(res->zwlr_output_configuration_v1_object)->q_func();
    return nullptr;
}

} // namespace Compositor

} // namespace Aurora
