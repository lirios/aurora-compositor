// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <LiriAuroraPlatform/liriauroraplatformglobal.h>

namespace Aurora {

namespace Platform {

class OutputsModelPrivate;

class LIRIAURORAPLATFORM_EXPORT OutputsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(PlatformOutputsModel)
    Q_DECLARE_PRIVATE(OutputsModel)
public:
    enum OutputRoles {
        OutputRole = Qt::UserRole + 1
    };

    explicit OutputsModel(QObject *parent = nullptr);
    ~OutputsModel();

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QScopedPointer<OutputsModelPrivate> const d_ptr;
};

} // namespace Platform

} // namespace Aurora
