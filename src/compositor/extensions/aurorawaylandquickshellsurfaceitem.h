// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/WaylandQuickItem>

namespace Aurora {

namespace Compositor {

class WaylandQuickShellSurfaceItemPrivate;
class WaylandShellSurface;

class LIRIAURORACOMPOSITOR_EXPORT WaylandQuickShellSurfaceItem : public WaylandQuickItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandQuickShellSurfaceItem)
    Q_PROPERTY(Aurora::Compositor::WaylandShellSurface *shellSurface READ shellSurface WRITE setShellSurface NOTIFY shellSurfaceChanged)
    Q_PROPERTY(QQuickItem *moveItem READ moveItem WRITE setMoveItem NOTIFY moveItemChanged)
    Q_PROPERTY(bool autoCreatePopupItems READ autoCreatePopupItems WRITE setAutoCreatePopupItems NOTIFY autoCreatePopupItemsChanged)
    Q_PROPERTY(bool staysOnTop READ staysOnTop WRITE setStaysOnTop NOTIFY staysOnTopChanged)
    Q_PROPERTY(bool staysOnBottom READ staysOnBottom WRITE setStaysOnBottom NOTIFY staysOnBottomChanged)
    Q_MOC_INCLUDE("aurorawaylandshellsurface.h")
    QML_NAMED_ELEMENT(ShellSurfaceItem)
    QML_ADDED_IN_VERSION(1, 0)
public:
    WaylandQuickShellSurfaceItem(QQuickItem *parent = nullptr);
    ~WaylandQuickShellSurfaceItem() override;

    WaylandShellSurface *shellSurface() const;
    void setShellSurface(WaylandShellSurface *shellSurface);

    QQuickItem *moveItem() const;
    void setMoveItem(QQuickItem *moveItem);

    bool autoCreatePopupItems();
    void setAutoCreatePopupItems(bool enabled);

    bool staysOnTop() const;
    void setStaysOnTop(bool on);
    bool staysOnBottom() const;
    void setStaysOnBottom(bool on);

Q_SIGNALS:
    void shellSurfaceChanged();
    void moveItemChanged();
    void autoCreatePopupItemsChanged();
    void staysOnTopChanged();
    void staysOnBottomChanged();

protected:
    WaylandQuickShellSurfaceItem(WaylandQuickShellSurfaceItemPrivate &dd, QQuickItem *parent);
};

} // namespace Compositor

} // namespace Aurora

