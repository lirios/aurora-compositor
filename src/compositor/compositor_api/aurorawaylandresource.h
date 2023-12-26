// Copyright (C) 2017 Klarälvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <QtCore/QObject>
#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/auroraqmlinclude.h>

struct wl_resource;

namespace Aurora {

namespace Compositor {

class LIRIAURORACOMPOSITOR_EXPORT WaylandResource
{
    Q_GADGET
    QML_NAMED_ELEMENT(waylandresource)
    QML_UNCREATABLE("")
    QML_ADDED_IN_VERSION(1, 0)
public:
    WaylandResource();
    explicit WaylandResource(wl_resource *resource);

    wl_resource *resource() const { return m_resource; }

private:
    wl_resource *m_resource = nullptr;
};

} // namespace Compositor

} // namespace Aurora

Q_DECLARE_METATYPE(Aurora::Compositor::WaylandResource)

