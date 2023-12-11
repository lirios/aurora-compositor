// Copyright (C) 2017 The Qt Company Ltd.
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

#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/QSGMaterialShader>
#include <QtQuick/QSGMaterial>

#include <LiriAuroraCompositor/WaylandQuickItem>
#include <LiriAuroraCompositor/WaylandOutput>

#include <QtCore/qpointer.h>

class QMutex;
class QOpenGLTexture;

namespace Aurora {

namespace Compositor {

class WaylandSurfaceTextureProvider;

#if QT_CONFIG(opengl)
class WaylandBufferMaterialShader : public QSGMaterialShader
{
public:
    WaylandBufferMaterialShader(WaylandBufferRef::BufferFormatEgl format);

    bool updateUniformData(RenderState &state,
                           QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
    void updateSampledImage(RenderState &state, int binding, QSGTexture **texture,
                            QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
    void setupExternalOESShader(const QString &shaderFilename);
};

class WaylandBufferMaterial : public QSGMaterial
{
public:
    WaylandBufferMaterial(WaylandBufferRef::BufferFormatEgl format);
    ~WaylandBufferMaterial() override;

    void setTextureForPlane(int plane, QOpenGLTexture *texture, QSGTexture *scenegraphTexture);
    void setBufferRef(WaylandQuickItem *surfaceItem, const WaylandBufferRef &ref);

    void bind();
    void updateScenegraphTextures(QRhi *rhi);

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode renderMode) const override;

private:
    friend WaylandBufferMaterialShader;

    void setTextureParameters(GLenum target);
    void ensureTextures(int count);

    const WaylandBufferRef::BufferFormatEgl m_format;
    QVarLengthArray<QOpenGLTexture*, 3> m_textures;
    QVarLengthArray<QSGTexture*, 3> m_scenegraphTextures;
    WaylandBufferRef m_bufferRef;
};
#endif // QT_CONFIG(opengl)

class WaylandQuickItemPrivate : public QQuickItemPrivate
{
    Q_DECLARE_PUBLIC(WaylandQuickItem)
public:
    WaylandQuickItemPrivate() = default;

    void init()
    {
        Q_Q(WaylandQuickItem);
        if (!mutex)
            mutex = new QMutex;

        view.reset(new WaylandView(q));
        q->setFlag(QQuickItem::ItemHasContents);

        q->update();

        q->setSmooth(true);

        setInputEventsEnabled(true);
        QObject::connect(q, &QQuickItem::windowChanged, q, &WaylandQuickItem::updateWindow);
        QObject::connect(view.data(), &WaylandView::surfaceChanged, q, &WaylandQuickItem::surfaceChanged);
        QObject::connect(view.data(), &WaylandView::surfaceChanged, q, &WaylandQuickItem::handleSurfaceChanged);
        QObject::connect(view.data(), &WaylandView::surfaceDestroyed, q, &WaylandQuickItem::surfaceDestroyed);
        QObject::connect(view.data(), &WaylandView::outputChanged, q, &WaylandQuickItem::outputChanged);
        QObject::connect(view.data(), &WaylandView::outputChanged, q, &WaylandQuickItem::updateOutput);
        QObject::connect(view.data(), &WaylandView::bufferLockedChanged, q, &WaylandQuickItem::bufferLockedChanged);
        QObject::connect(view.data(), &WaylandView::allowDiscardFrontBufferChanged, q, &WaylandQuickItem::allowDiscardFrontBuffer);

        q->updateWindow();
    }

    static const WaylandQuickItemPrivate* get(const WaylandQuickItem *item) { return item->d_func(); }

    void setInputEventsEnabled(bool enable)
    {
        Q_Q(WaylandQuickItem);
        q->setAcceptedMouseButtons(enable ? (Qt::LeftButton | Qt::MiddleButton | Qt::RightButton |
                                   Qt::ExtraButton1 | Qt::ExtraButton2 | Qt::ExtraButton3 | Qt::ExtraButton4 |
                                   Qt::ExtraButton5 | Qt::ExtraButton6 | Qt::ExtraButton7 | Qt::ExtraButton8 |
                                   Qt::ExtraButton9 | Qt::ExtraButton10 | Qt::ExtraButton11 |
                                   Qt::ExtraButton12 | Qt::ExtraButton13) : Qt::NoButton);
        q->setAcceptTouchEvents(enable);
        q->setAcceptHoverEvents(enable);
        inputEventsEnabled = enable;
    }

    bool shouldSendInputEvents() const { return view->surface() && inputEventsEnabled; }
    qreal scaleFactor() const;

    WaylandQuickItem *findSibling(WaylandSurface *surface) const;
    void placeAboveSibling(WaylandQuickItem *sibling);
    void placeBelowSibling(WaylandQuickItem *sibling);
    void placeAboveParent();
    void placeBelowParent();

    virtual void raise();
    virtual void lower();

    static QMutex *mutex;

    QScopedPointer<WaylandView> view;
    QPointer<WaylandSurface> oldSurface;
    mutable WaylandSurfaceTextureProvider *provider = nullptr;
    QMetaObject::Connection texProviderConnection;
    bool paintEnabled = true;
    bool touchEventsEnabled = true;
    bool inputEventsEnabled = true;
    bool isDragging = false;
    bool newTexture = false;
    bool focusOnClick = true;
    bool belowParent = false;
#if QT_CONFIG(opengl)
    bool paintByProvider = false;
#endif
    QPointF hoverPos;
    QMatrix4x4 lastMatrix;

    QQuickWindow *connectedWindow = nullptr;
    WaylandOutput *connectedOutput = nullptr;
    WaylandSurface::Origin origin = WaylandSurface::OriginTopLeft;
    QPointer<QObject> subsurfaceHandler;
    QList<WaylandSeat *> touchingSeats;
};

} // namespace Compositor

} // namespace Aurora

