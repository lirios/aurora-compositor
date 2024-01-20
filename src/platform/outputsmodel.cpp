// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "eglfsdeviceintegration_p.h"
#include "outputsmodel.h"
#include "outputsmodel_p.h"

namespace Aurora {

namespace Platform {

/*
 * PlatformOutputsModel
 */

OutputsModel::OutputsModel(QObject *parent)
    : QAbstractListModel(parent)
    , d_ptr(new OutputsModelPrivate(this))
{
    Q_D(OutputsModel);

    auto *deviceIntegration = auroraDeviceIntegration();
    d->outputs = deviceIntegration->outputs();

    connect(deviceIntegration, &DeviceIntegration::outputAdded, this, [this, d](Output *output) {
        beginInsertRows(QModelIndex(), d->outputs.size(), d->outputs.size());
        d->outputs.append(output);
        endInsertRows();
    });
    connect(deviceIntegration, &DeviceIntegration::outputRemoved, this, [this, d](Output *output) {
        beginRemoveRows(QModelIndex(), d->outputs.size(), d->outputs.size());
        d->outputs.removeOne(output);
        endRemoveRows();
    });
}

OutputsModel::~OutputsModel()
{
}

QHash<int, QByteArray> OutputsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[OutputRole] = "output";
    return roles;
}

int OutputsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    Q_D(const OutputsModel);
    return d->outputs.size();
}

QVariant OutputsModel::data(const QModelIndex &index, int role) const
{
    Q_D(const OutputsModel);

    if (index.row() < 0 || index.row() >= d->outputs.size())
        return {};

    auto *output = d->outputs.at(index.row());
    if (!output)
        return {};

    switch (role) {
    case OutputRole:
        return QVariant::fromValue(output);
    default:
        break;
    }

    return {};
}

/*
 * OutputsModelPrivate
 */

OutputsModelPrivate::OutputsModelPrivate(OutputsModel *self)
    : q_ptr(self)
{
}

} // namespace Platform

} // namespace Aurora
