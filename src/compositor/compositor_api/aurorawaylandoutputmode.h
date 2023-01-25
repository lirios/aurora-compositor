// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDOUTPUTMODE_H
#define AURORA_COMPOSITOR_WAYLANDOUTPUTMODE_H

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <QtCore/QSize>

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandOutputMode
{
public:
    explicit WaylandOutputMode();
    WaylandOutputMode(const QSize &size, int refreshRate);
    WaylandOutputMode(const WaylandOutputMode &other);
    ~WaylandOutputMode();

    WaylandOutputMode &operator=(const WaylandOutputMode &other);
    bool operator==(const WaylandOutputMode &other) const;
    bool operator!=(const WaylandOutputMode &other) const;

    bool isValid() const;

    QSize size() const;
    int refreshRate() const;

private:
    class WaylandOutputModePrivate *const d;
    friend class WaylandOutputPrivate;

    void setSize(const QSize &size);
};

} // namespace Compositor

} // namespace Aurora

Q_DECLARE_TYPEINFO(Aurora::Compositor::WaylandOutputMode, Q_MOVABLE_TYPE);

#endif // AURORA_COMPOSITOR_WAYLANDOUTPUTMODE_H
