// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Aurora API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <LiriAuroraPlatform/Output>

namespace Aurora {

namespace Platform {

class LIRIAURORAPLATFORM_EXPORT OutputPrivate
{
    Q_DECLARE_PUBLIC(Output)
public:
    explicit OutputPrivate(Output *self);

    static OutputPrivate *get(Output *output)
    {
        return output->d_func();
    }

    void setGlobalPosition(const QPoint &globalPosition);
    void setScale(qreal scale);

    QUuid uuid;
    QScreen *screen = nullptr;
    bool enabled = true;
    QPoint globalPosition;
    qreal scale = 1.0f;
    int depth = 0;
    QImage::Format format = QImage::Format_Invalid;
    Output::PowerState powerState = Output::PowerState::On;
    Output::ContentType contentType = Output::ContentType::Unknown;
    QList<Output::Mode> modes;
    QList<Output::Mode>::iterator currentMode = modes.end();

    Output::Information information;
    Output::State state;

protected:
    Output *q_ptr = nullptr;
};

} // namespace Platform

} // namespace Aurora
