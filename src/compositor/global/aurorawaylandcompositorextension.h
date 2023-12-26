// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/auroraqmlinclude.h>

#include <QtCore/QObject>

struct wl_interface;

namespace Aurora {

namespace Compositor {

class WaylandCompositor;
class WaylandCompositorExtension;
class WaylandCompositorExtensionPrivate;

class LIRIAURORACOMPOSITOR_EXPORT WaylandObject : public QObject
{
    Q_OBJECT
public:
    ~WaylandObject() override;

    WaylandCompositorExtension *extension(const QByteArray &name);
    WaylandCompositorExtension *extension(const wl_interface *interface);
    QList<WaylandCompositorExtension *> extensions() const;
    void addExtension(WaylandCompositorExtension *extension);
    void removeExtension(WaylandCompositorExtension *extension);

protected:
    WaylandObject(QObject *parent = nullptr);
    WaylandObject(QObjectPrivate &d, QObject *parent = nullptr);
    QList<WaylandCompositorExtension *> extension_vector;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandCompositorExtension : public WaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandCompositorExtension)
    QML_NAMED_ELEMENT(WaylandExtension)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("")
public:
    WaylandCompositorExtension();
    WaylandCompositorExtension(WaylandObject *container);
    ~WaylandCompositorExtension() override;

    WaylandObject *extensionContainer() const;
    void setExtensionContainer(WaylandObject *container);

    virtual void initialize();
    bool isInitialized() const;

    virtual const struct wl_interface *extensionInterface() const = 0;

protected:
    WaylandCompositorExtension(WaylandCompositorExtensionPrivate &dd);
    WaylandCompositorExtension(WaylandObject *container, WaylandCompositorExtensionPrivate &dd);

    bool event(QEvent *event) override;
};

template <typename T>
class LIRIAURORACOMPOSITOR_EXPORT WaylandCompositorExtensionTemplate : public WaylandCompositorExtension
{
public:
    WaylandCompositorExtensionTemplate()
        : WaylandCompositorExtension()
    { }

    WaylandCompositorExtensionTemplate(WaylandObject *container)
        : WaylandCompositorExtension(container)
    { }

    const struct wl_interface *extensionInterface() const override
    {
        return T::interface();
    }

    static T *findIn(WaylandObject *container)
    {
        if (!container) return nullptr;
        return qobject_cast<T *>(container->extension(T::interfaceName()));
    }

protected:
    WaylandCompositorExtensionTemplate(WaylandCompositorExtensionPrivate &dd)
        : WaylandCompositorExtension(dd)
    { }

    WaylandCompositorExtensionTemplate(WaylandObject *container, WaylandCompositorExtensionPrivate &dd)
        : WaylandCompositorExtension(container,dd)
    { }
};

} // namespace Compositor

} // namespace Aurora

