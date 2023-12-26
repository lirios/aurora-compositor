// Copyright (C) 2023 The Qt Company Ltd.
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

#include <QObject>
#include <QMutex>
#include <QLoggingCategory>
#include <LiriAuroraCompositor/liriauroracompositorglobal.h>

class QOpenGLContext;
class QOpenGLTexture;

namespace Aurora {

namespace Compositor {

Q_DECLARE_LOGGING_CATEGORY(qLcWTO)

namespace Internal {

class LIRIAURORACOMPOSITOR_EXPORT WaylandTextureOrphanage : public QObject
{
    Q_OBJECT

public:
    WaylandTextureOrphanage(){};
    ~WaylandTextureOrphanage();

    static WaylandTextureOrphanage *instance();

    // texture that isn't needed anymore will be "take care of" (killed) appropriately
    void admitTexture(QOpenGLTexture *tex, QOpenGLContext *ctx);

    // uses QOpenGLContext::currentContext to call deleteTexturesByContext on all shared ctx
    void deleteTextures();

public Q_SLOTS:
    // uses sender() to call deleteTexturesByContext
    void onContextAboutToBeDestroyed(QOpenGLContext *ctx);

private:
    void deleteTexturesByContext(QOpenGLContext *ctx);

    // tracks all the orphanes that need to be deleted
    QMultiHash<QOpenGLContext *, QOpenGLTexture *> m_orphanedTextures;

    QMutex m_containerLock;
};

} // namespace Internal

} // namespace Compositor

} // namespace Aurora
