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

#ifndef AURORA_COMPOSITOR_WAYLANDQUICKITEM_P_H
#define AURORA_COMPOSITOR_WAYLANDQUICKITEM_P_H

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

#include <QtCore/QMutex>
#include <QtQuick/QSGMaterialShader>
#include <QtQuick/QSGMaterial>
#include <QtQuick/QQuickWindow>

#include <LiriAuroraCompositor/WaylandQuickItem>
#include <LiriAuroraCompositor/WaylandOutput>

class QOpenGLTexture;

namespace Aurora {

namespace Compositor {

class WaylandSurfaceTextureProvider;

#if QT_CONFIG(opengl)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
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
#else
class WaylandBufferMaterialShader : public QSGMaterialShader
{
public:
    WaylandBufferMaterialShader(WaylandBufferRef::BufferFormatEgl format);

    void updateState(const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect) override;
    char const *const *attributeNames() const override;

protected:
    void initialize() override;

private:
    const WaylandBufferRef::BufferFormatEgl m_format;
    int m_id_matrix;
    int m_id_opacity;
    QVarLengthArray<int, 3> m_id_tex;
};

class WaylandBufferMaterial : public QSGMaterial
{
public:
    WaylandBufferMaterial(WaylandBufferRef::BufferFormatEgl format);
    ~WaylandBufferMaterial() override;

    void setTextureForPlane(int plane, QOpenGLTexture *texture);

    void bind();

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader() const override;

private:
    void setTextureParameters(GLenum target);
    void ensureTextures(int count);

    const WaylandBufferRef::BufferFormatEgl m_format;
    QVarLengthArray<QOpenGLTexture*, 3> m_textures;
};
#endif
#endif // QT_CONFIG(opengl)

class WaylandQuickItemPrivate
{
    Q_DECLARE_PUBLIC(WaylandQuickItem)
public:
    WaylandQuickItemPrivate(WaylandQuickItem *self)
        : q_ptr(self)
    {
    }

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
        QObject::connect(view.data(), &WaylandView::allowDiscardFrontBufferChanged, q, &WaylandQuickItem::allowDiscardFrontBufferChanged);

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

private:
    WaylandQuickItem *q_ptr = nullptr;
};

} // namespace Compositor

} // namespace Aurora

#endif  /*QWAYLANDQUICKITEM_P_H*/
