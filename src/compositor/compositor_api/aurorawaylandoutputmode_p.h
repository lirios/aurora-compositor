// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDOUTPUTMODE_P_H
#define AURORA_COMPOSITOR_WAYLANDOUTPUTMODE_P_H

#include <LiriAuroraCompositor/WaylandOutput>

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

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandOutputModePrivate
{
public:
    WaylandOutputModePrivate() {}

    QSize size;
    int refreshRate = 60000;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDOUTPUTMODE_P_H
