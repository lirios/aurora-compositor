// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <LiriAuroraCompositor/WaylandCompositorExtension>

QT_FORWARD_DECLARE_CLASS(QQuickItem)

namespace Aurora {

namespace Compositor {

class WaylandOutput;
class WaylandWlrScreencopyManagerV1Private;
class WaylandWlrScreencopyFrameV1;
class WaylandWlrScreencopyFrameV1Private;

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrScreencopyManagerV1
        : public WaylandCompositorExtensionTemplate<WaylandWlrScreencopyManagerV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrScreencopyManagerV1)
public:
    WaylandWlrScreencopyManagerV1();
    WaylandWlrScreencopyManagerV1(WaylandCompositor *compositor);

    void initialize() override;

    static const wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void captureOutputRequested(Aurora::Compositor::WaylandWlrScreencopyFrameV1 *frame);
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrScreencopyFrameV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrScreencopyFrameV1)
    Q_PROPERTY(bool overlayCursor READ overlayCursor CONSTANT)
    Q_PROPERTY(Aurora::Compositor::WaylandOutput *output READ output CONSTANT)
    Q_PROPERTY(QRect region READ region CONSTANT)
    Q_PROPERTY(Aurora::Compositor::WaylandWlrScreencopyFrameV1::Flags flags READ flags WRITE setFlags NOTIFY flagsChanged)
public:
    enum Flag {
        YInvert = 1
    };
    Q_ENUM(Flag)
    Q_DECLARE_FLAGS(Flags, Flag)

    ~WaylandWlrScreencopyFrameV1();

    bool overlayCursor() const;
    WaylandOutput *output() const;
    QRect region() const;

    Flags flags() const;
    void setFlags(Flags flags);

    Q_INVOKABLE void copy(const QString &childToCapture = QString());

Q_SIGNALS:
    void flagsChanged();
    void ready();

private:
    explicit WaylandWlrScreencopyFrameV1(QObject *parent = nullptr);

    friend class WaylandWlrScreencopyManagerV1Private;
};

} // namespace Compositor

} // namespace Aurora

Q_DECLARE_OPERATORS_FOR_FLAGS(Aurora::Compositor::WaylandWlrScreencopyFrameV1::Flags)

