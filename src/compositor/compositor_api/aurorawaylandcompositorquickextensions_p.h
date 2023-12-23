/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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

#include <QtQml/qqml.h>
#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/aurorawaylandquickextension.h>

#include <LiriAuroraCompositor/aurorawaylandcompositor.h>
#include <LiriAuroraCompositor/aurorawaylandquickcompositor.h>
#include <LiriAuroraCompositor/aurorawaylandqtwindowmanager.h>
#include <LiriAuroraCompositor/aurorawaylandtextinputmanager.h>
#include <QtCore/private/qglobal_p.h>
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
#include <LiriAuroraCompositor/aurorawaylandtextinputmanagerv4.h>
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
#include <LiriAuroraCompositor/aurorawaylandqttextinputmethodmanager.h>
#include <LiriAuroraCompositor/aurorawaylandidleinhibitv1.h>

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickCompositorQuickExtensionContainer : public WaylandQuickCompositor
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Aurora::Compositor::WaylandCompositorExtension> extensions READ extensions)
    Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false)
    Q_CLASSINFO("DefaultProperty", "data")
    QML_NAMED_ELEMENT(WaylandCompositor)
    QML_ADDED_IN_VERSION(1, 0)
public:
    QQmlListProperty<QObject> data()
    {
        return QQmlListProperty<QObject>(this, &m_objects);
    }

    QQmlListProperty<WaylandCompositorExtension> extensions()
    {
        return QQmlListProperty<WaylandCompositorExtension>(this, this,
                                                   &append_extension,
                                                   &countFunction,
                                                   &atFunction,
                                                   &clearFunction);
    }

    static qsizetype countFunction(QQmlListProperty<WaylandCompositorExtension> *list)
    {
        return static_cast<WaylandQuickCompositorQuickExtensionContainer *>(list->data)->extension_vector.size();
    }

    static WaylandCompositorExtension *atFunction(QQmlListProperty<WaylandCompositorExtension> *list, qsizetype index)
    {
        return static_cast<WaylandQuickCompositorQuickExtensionContainer *>(list->data)->extension_vector.at(index);
    }

    static void append_extension(QQmlListProperty<WaylandCompositorExtension> *list, WaylandCompositorExtension *extension)
    {
        WaylandQuickCompositorQuickExtensionContainer *quickExtObj = static_cast<WaylandQuickCompositorQuickExtensionContainer *>(list->data);
        extension->setExtensionContainer(quickExtObj);
    }

    static void clearFunction(QQmlListProperty<WaylandCompositorExtension> *list)
    {
        static_cast<WaylandQuickCompositorQuickExtensionContainer *>(list->data)->extension_vector.clear();
    }

private:
    QList<QObject *> m_objects;
};


// Note: These have to be in a header with a Q_OBJECT macro, otherwise we won't run moc on it
AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS(WaylandQtWindowManager, QtWindowManager)
AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS(WaylandIdleInhibitManagerV1, IdleInhibitManagerV1)
AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS(WaylandTextInputManager, TextInputManager)
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS(WaylandTextInputManagerV4, TextInputManagerV4)
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS(WaylandQtTextInputMethodManager, QtTextInputMethodManager)

} // namespace Compositor

} // namespace Aurora

