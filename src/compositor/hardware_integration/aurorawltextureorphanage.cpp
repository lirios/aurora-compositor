// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "aurorawltextureorphanage_p.h"

#include <QOpenGLContext>
#include <QOpenGLTexture>
#include <QDebug>
#include <QtTypeTraits>
#include <QMutexLocker>

namespace Aurora {

namespace Compositor {

Q_LOGGING_CATEGORY(qLcWTO, "qt.waylandcompositor.orphanage")

Q_GLOBAL_STATIC(Internal::WaylandTextureOrphanage, inst)

namespace Internal {

WaylandTextureOrphanage::~WaylandTextureOrphanage()
{
    QMutexLocker locker(&m_containerLock);
    if (!m_orphanedTextures.isEmpty()) {
        qCWarning(qLcWTO) << Q_FUNC_INFO << "m_orphanedTextures container isn't empty! content:"
                          << m_orphanedTextures;
    }
}

WaylandTextureOrphanage *WaylandTextureOrphanage::instance()
{
    return inst;
}

void WaylandTextureOrphanage::admitTexture(QOpenGLTexture *tex, QOpenGLContext *ctx)
{
    qCDebug(qLcWTO) << Q_FUNC_INFO << "got a texture (" << (void *)tex
                    << ") ready to be deleted! It's ctx:" << ctx;

    {
        QMutexLocker locker(&m_containerLock);
        m_orphanedTextures.insert(ctx, tex);
    }

    connect(ctx, &QOpenGLContext::aboutToBeDestroyed, this,
            [this, ctx]() { this->onContextAboutToBeDestroyed(ctx); },
            Qt::ConnectionType(Qt::DirectConnection));
}

void WaylandTextureOrphanage::deleteTextures()
{
    QOpenGLContext *cCtx = QOpenGLContext::currentContext();

    if (cCtx == nullptr) {
        qCWarning(qLcWTO) << Q_FUNC_INFO << "cannot delete textures without current OpenGL context";
        return;
    }

    {
        QMutexLocker locker(&m_containerLock);

        for (QOpenGLContext *aCtx : m_orphanedTextures.keys()) {
            if (QOpenGLContext::areSharing(cCtx, aCtx)) {

                qCDebug(qLcWTO) << Q_FUNC_INFO << "currentContext (" << cCtx
                                << ") and ctx of orphane(s) (" << aCtx
                                << ") are shared! => deleteTexturesByContext";

                deleteTexturesByContext(aCtx);
            }
        }
    }
}

void WaylandTextureOrphanage::onContextAboutToBeDestroyed(QOpenGLContext *ctx)
{
    Q_ASSERT(ctx != nullptr);

    qCDebug(qLcWTO) << Q_FUNC_INFO << " ctx (" << ctx
                    << ") fired aboutToBeDestroyed => deleteTexturesByContext(ctx)";

    {
        QMutexLocker locker(&m_containerLock);
        deleteTexturesByContext(ctx);
    }
}

void WaylandTextureOrphanage::deleteTexturesByContext(QOpenGLContext *ctx)
{
    // NOTE: We are (by class-internal design) locked (m_containerLock)
    // when we enter this function!
    // If not (e.g.: someone changes something in/around this class),
    // then in a debug-build we will fail below:
    Q_ASSERT(!m_containerLock.tryLock());

    QList<QOpenGLTexture *> texturesToDelete = m_orphanedTextures.values(ctx);
    m_orphanedTextures.remove(ctx);

    for (QOpenGLTexture *tex : texturesToDelete) {
        delete tex;
        qCDebug(qLcWTO) << Q_FUNC_INFO << " texture (" << (void *)tex << ") got deleted";
    }
}

} // namespace Internal

} // namespace Compositor

} // namespace Aurora

#include "moc_aurorawltextureorphanage_p.cpp"
