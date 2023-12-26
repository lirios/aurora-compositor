// Copyright (C) 2017 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#pragma once

#include <LiriAuroraCompositor/WaylandBufferRef>
#include <LiriAuroraCompositor/liriauroracompositorglobal.h>

#include <QtCore/QPointF>
#include <QtCore/QObject>

namespace Aurora {

namespace Compositor {

class WaylandSurface;
class WaylandViewPrivate;
class WaylandOutput;

class LIRIAURORACOMPOSITOR_EXPORT WaylandView : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandView)
    Q_PROPERTY(QObject *renderObject READ renderObject CONSTANT)
    Q_PROPERTY(Aurora::Compositor::WaylandSurface *surface READ surface WRITE setSurface NOTIFY surfaceChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandOutput *output READ output WRITE setOutput NOTIFY outputChanged)
    Q_PROPERTY(bool bufferLocked READ isBufferLocked WRITE setBufferLocked NOTIFY bufferLockedChanged)
    Q_PROPERTY(bool allowDiscardFrontBuffer READ allowDiscardFrontBuffer WRITE setAllowDiscardFrontBuffer NOTIFY allowDiscardFrontBufferChanged)
    Q_MOC_INCLUDE("aurorawaylandoutput.h")
public:
    WaylandView(QObject *renderObject = nullptr, QObject *parent = nullptr);
    ~WaylandView() override;

    QObject *renderObject() const;

    WaylandSurface *surface() const;
    void setSurface(WaylandSurface *surface);

    WaylandOutput *output() const;
    void setOutput(WaylandOutput *output);

    virtual void bufferCommitted(const WaylandBufferRef &buffer, const QRegion &damage);
    virtual bool advance();
    virtual void discardCurrentBuffer();
    virtual WaylandBufferRef currentBuffer();
    virtual QRegion currentDamage();

    bool isBufferLocked() const;
    void setBufferLocked(bool locked);

    bool allowDiscardFrontBuffer() const;
    void setAllowDiscardFrontBuffer(bool discard);

    void setPrimary();
    bool isPrimary() const;

    struct wl_resource *surfaceResource() const;

Q_SIGNALS:
    void surfaceChanged();
    void surfaceDestroyed();
    void outputChanged();
    void bufferLockedChanged();
    void allowDiscardFrontBufferChanged();
};

} // namespace Compositor

} // namespace Aurora

