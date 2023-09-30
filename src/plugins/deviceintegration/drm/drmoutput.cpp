// SPDX-FileCopyrightText: 2024 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drmoutput.h"

namespace Aurora {

namespace Platform {

DrmOutput::DrmOutput(QObject *parent)
    : Output(parent)
{
}

DrmOutput::~DrmOutput()
{
    destroy();
}

QString DrmOutput::name() const
{
    return QString();
}

QString DrmOutput::description() const
{
    return QString();
}

void DrmOutput::destroy()
{
}

} // namespace Platform

} // namespace Aurora
