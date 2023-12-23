/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include <QtCore/QObject>

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/aurorawaylandqmlinclude.h>

struct wl_interface;

namespace Aurora {

namespace Compositor {

class WaylandCompositor;
class WaylandCompositorExtension;
class WaylandCompositorExtensionPrivate;
class WaylandObjectPrivate;

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

    QList<WaylandCompositorExtension *> extension_vector;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandCompositorExtension : public WaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandCompositorExtension)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(WaylandExtension)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("")
#endif
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
    WaylandCompositorExtension(WaylandCompositorExtensionPrivate &dptr);
    WaylandCompositorExtension(WaylandObject *container, WaylandCompositorExtensionPrivate &dptr);

    bool event(QEvent *event) override;

protected:
    QScopedPointer<WaylandCompositorExtensionPrivate> const d_ptr;
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
        : WaylandCompositorExtension(container, dd)
    { }
};

} // namespace Compositor

} // namespace Aurora

