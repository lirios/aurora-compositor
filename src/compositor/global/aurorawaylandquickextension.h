// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <QtQml/QQmlParserStatus>
#include <QtQml/QQmlListProperty>

namespace Aurora {

namespace Compositor {

#define AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(className) \
    class LIRIAURORACOMPOSITOR_EXPORT className##QuickExtension : public className, public QQmlParserStatus \
    { \
/* qmake ignore Q_OBJECT */ \
        Q_OBJECT \
        Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false) \
        Q_CLASSINFO("DefaultProperty", "data") \
        Q_INTERFACES(QQmlParserStatus) \
    public: \
        QQmlListProperty<QObject> data() \
        { \
            return QQmlListProperty<QObject>(this, &m_objects); \
        } \
        void classBegin() override {} \
        void componentComplete() override { if (!isInitialized()) initialize(); } \
    private: \
        QList<QObject *> m_objects; \
    };

#define AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_CONTAINER_CLASS(className) \
    class LIRIAURORACOMPOSITOR_EXPORT className##QuickExtensionContainer : public className \
    { \
/* qmake ignore Q_OBJECT */ \
        Q_OBJECT \
        Q_PROPERTY(QQmlListProperty<Aurora::Compositor::WaylandCompositorExtension> extensions READ extensions) \
        Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false) \
        Q_CLASSINFO("DefaultProperty", "data") \
    public: \
        QQmlListProperty<QObject> data() \
        { \
            return QQmlListProperty<QObject>(this, &m_objects); \
        } \
        QQmlListProperty<WaylandCompositorExtension> extensions() \
        { \
            return QQmlListProperty<WaylandCompositorExtension>(this, this, \
                                                       &className##QuickExtensionContainer::append_extension, \
                                                       &className##QuickExtensionContainer::countFunction, \
                                                       &className##QuickExtensionContainer::atFunction, \
                                                       &className##QuickExtensionContainer::clearFunction); \
        } \
        static int countFunction(QQmlListProperty<WaylandCompositorExtension> *list) \
        { \
            return static_cast<className##QuickExtensionContainer *>(list->data)->extension_vector.size(); \
        } \
        static WaylandCompositorExtension *atFunction(QQmlListProperty<WaylandCompositorExtension> *list, int index) \
        { \
            return static_cast<className##QuickExtensionContainer *>(list->data)->extension_vector.at(index); \
        } \
        static void append_extension(QQmlListProperty<WaylandCompositorExtension> *list, WaylandCompositorExtension *extension) \
        { \
            className##QuickExtensionContainer *quickExtObj = static_cast<className##QuickExtensionContainer *>(list->data); \
            extension->setExtensionContainer(quickExtObj); \
        } \
        static void clearFunction(QQmlListProperty<WaylandCompositorExtension> *list) \
        { \
            static_cast<className##QuickExtensionContainer *>(list->data)->extension_vector.clear(); \
        } \
    private: \
        QList<QObject *> m_objects; \
    };

#define AURORA_COMPOSITOR_DECLARE_QUICK_EXTENSION_NAMED_CLASS(className, QmlType) \
    class LIRIAURORACOMPOSITOR_EXPORT className##QuickExtension : public className, public QQmlParserStatus \
    { \
/* qmake ignore Q_OBJECT */ \
        Q_OBJECT \
        Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false) \
        Q_CLASSINFO("DefaultProperty", "data") \
        Q_INTERFACES(QQmlParserStatus) \
        QML_NAMED_ELEMENT(QmlType) \
        QML_ADDED_IN_VERSION(1, 0) \
    public: \
        QQmlListProperty<QObject> data() \
        { \
            return QQmlListProperty<QObject>(this, &m_objects); \
        } \
        void classBegin() override {} \
        void componentComplete() override { if (!isInitialized()) initialize(); } \
    private: \
        QList<QObject *> m_objects; \
    };

} // namespace Compositor

} // namespace Aurora

