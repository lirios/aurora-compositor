// Copyright (C) 2021 LG Electronics Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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

#include <QObject>
#include <LiriAuroraCompositor/aurorawaylandcompositorextension.h>
#include <QtCore/private/qglobal_p.h>

class QQuickWindow;

namespace Aurora {

namespace Compositor {

class WaylandPresentationTimePrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandPresentationTime : public WaylandCompositorExtensionTemplate<WaylandPresentationTime>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandPresentationTime)
public:
    WaylandPresentationTime();
    WaylandPresentationTime(WaylandCompositor *compositor);

    WaylandCompositor *compositor() const;
    void initialize() override;

    Q_INVOKABLE void sendFeedback(QQuickWindow *window, quint64 sequence, quint64 tv_sec, quint32 tv_nsec);

    static const struct wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void presented(quint64 sequence, quint64 tv_sec, quint32 tv_nsec, quint32 refresh_nsec);
};

} // namespace Compositor

} // namespace Aurora

