/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef AURORA_COMPOSITOR_WAYLANDQUICKHARDWARELAYER_P_H
#define AURORA_COMPOSITOR_WAYLANDQUICKHARDWARELAYER_P_H

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

#include <LiriAuroraCompositor/WaylandQuickItem>

namespace Aurora {

namespace Compositor {

class WaylandQuickHardwareLayerPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickHardwareLayer : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_DECLARE_PRIVATE(WaylandQuickHardwareLayer)
    Q_PROPERTY(int stackingLevel READ stackingLevel WRITE setStackingLevel NOTIFY stackingLevelChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(WaylandHardwareLayer)
    QML_ADDED_IN_VERSION(1, 2)
#endif
public:
    explicit WaylandQuickHardwareLayer(QObject *parent = nullptr);
    ~WaylandQuickHardwareLayer() override;

    int stackingLevel() const;
    void setStackingLevel(int level);

    WaylandQuickItem *waylandItem() const;

    void classBegin() override;
    void componentComplete() override;

    void setSceneGraphPainting(bool);
    void initialize();

Q_SIGNALS:
    void stackingLevelChanged();
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDQUICKHARDWARELAYER_P_H
