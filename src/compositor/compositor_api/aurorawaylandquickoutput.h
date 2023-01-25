// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDQUICKOUTPUT_H
#define AURORA_COMPOSITOR_WAYLANDQUICKOUTPUT_H

#include <QtQuick/QQuickWindow>
#include <LiriAuroraCompositor/aurorawaylandoutput.h>
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>

class QQuickWindow;

namespace Aurora {

namespace Compositor {

class WaylandQuickCompositor;

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickOutput : public WaylandOutput, public QQmlParserStatus
{
    Q_INTERFACES(QQmlParserStatus)
    Q_OBJECT
    AURORA_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandQuickOutput)
    Q_PROPERTY(bool automaticFrameCallback READ automaticFrameCallback WRITE setAutomaticFrameCallback NOTIFY automaticFrameCallbackChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(WaylandOutput)
    QML_ADDED_IN_VERSION(1, 0)
#endif
public:
    WaylandQuickOutput();
    WaylandQuickOutput(WaylandCompositor *compositor, QWindow *window);

    void update() override;

    bool automaticFrameCallback() const;
    void setAutomaticFrameCallback(bool automatic);

    QQuickItem *pickClickableItem(const QPointF &position);

public Q_SLOTS:
    void updateStarted();

Q_SIGNALS:
    void automaticFrameCallbackChanged();

protected:
    void initialize() override;
    void classBegin() override;
    void componentComplete() override;

private:
    void doFrameCallbacks();

    bool m_updateScheduled = false;
    bool m_automaticFrameCallback = true;
};

} // namespace Compositor

} // namespace Aurora

#endif
