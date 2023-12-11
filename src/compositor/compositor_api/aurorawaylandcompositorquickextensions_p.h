// Copyright (C) 2020 The Qt Company Ltd.
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

#include <QtQml/qqml.h>
#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/aurorawaylandquickextension.h>

#include <LiriAuroraCompositor/aurorawaylandcompositor.h>
#include <LiriAuroraCompositor/aurorawaylandquickcompositor.h>
#include <LiriAuroraCompositor/aurorawaylandqtwindowmanager.h>
#include <LiriAuroraCompositor/aurorawaylandtextinputmanager.h>
#include <QtCore/private/qglobal_p.h>
#include <LiriAuroraCompositor/aurorawaylandtextinputmanagerv3.h>
#include <LiriAuroraCompositor/aurorawaylandqttextinputmethodmanager.h>
#include <LiriAuroraCompositor/aurorawaylandidleinhibitv1.h>

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickCompositorQuickExtensionContainer : public WaylandQuickCompositor
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<WaylandCompositorExtension> extensions READ extensions)
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
AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS(WaylandTextInputManagerV3, TextInputManagerV3)
AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS(WaylandQtTextInputMethodManager, QtTextInputMethodManager)

} // namespace Compositor

} // namespace Aurora

