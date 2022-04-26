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

#include "aurorawaylandquickitem.h"
#include "aurorawaylandquickitem_p.h"
#include "aurorawaylandquicksurface.h"
#include "aurorawaylandinputmethodcontrol.h"
#include "aurorawaylandtextinput.h"
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
#include "aurorawaylandtextinputv4.h"
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP
#include "aurorawaylandqttextinputmethod.h"
#include "aurorawaylandquickoutput.h"
#include <LiriAuroraCompositor/aurorawaylandcompositor.h>
#include <LiriAuroraCompositor/aurorawaylandseat.h>
#include <LiriAuroraCompositor/aurorawaylandbufferref.h>
#include <LiriAuroraCompositor/WaylandDrag>
#include <LiriAuroraCompositor/private/aurorawlclientbufferintegration_p.h>
#include <LiriAuroraCompositor/private/aurorawaylandsurface_p.h>

#if QT_CONFIG(opengl)
#  if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#    include <QtOpenGL/QOpenGLTexture>
#else
#    include <QtGui/QOpenGLTexture>
#  endif
#  include <QtGui/QOpenGLFunctions>
#endif

#include <QtGui/QKeyEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>
#include <QtGui/QScreen>

#include <QtQuick/QSGSimpleTextureNode>
#include <QtQuick/QQuickWindow>
#include <QtQuick/qsgtexture.h>

#include <QtCore/QFile>
#include <QtCore/QMutexLocker>
#include <QtCore/QMutex>

#include <wayland-server-core.h>
#include <QThread>

#if QT_CONFIG(opengl)
#include <QtGui/private/qshaderdescription_p_p.h>
#endif

#ifndef GL_TEXTURE_EXTERNAL_OES
#define GL_TEXTURE_EXTERNAL_OES 0x8D65
#endif

namespace Aurora {

namespace Compositor {


#if QT_CONFIG(opengl)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
static const struct {
    const char * const vertexShaderSourceFile;
    const char * const fragmentShaderSourceFile;
    GLenum textureTarget;
    int planeCount;
    bool canProvideTexture;
    QSGMaterial::Flags materialFlags;
    QSGMaterialType materialType;
} bufferTypes[] = {
    // BufferFormatEgl_Null
    { "", "", 0, 0, false, {}, {} },

    // BufferFormatEgl_RGB
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert.qsb",
        ":/qt-project.org/wayland/compositor/shaders/surface_rgbx.frag.qsb",
        GL_TEXTURE_2D, 1, true,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_RGBA
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert.qsb",
        ":/qt-project.org/wayland/compositor/shaders/surface_rgba.frag.qsb",
        GL_TEXTURE_2D, 1, true,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_EXTERNAL_OES
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert.qsb",
        ":/qt-project.org/wayland/compositor/shaders/surface_oes_external.frag",
        GL_TEXTURE_EXTERNAL_OES, 1, false,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_Y_U_V
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert.qsb",
        ":/qt-project.org/wayland/compositor/shaders/surface_y_u_v.frag.qsb",
        GL_TEXTURE_2D, 3, false,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_Y_UV
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert.qsb",
        ":/qt-project.org/wayland/compositor/shaders/surface_y_uv.frag.qsb",
        GL_TEXTURE_2D, 2, false,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_Y_XUXV
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert.qsb",
        ":/qt-project.org/wayland/compositor/shaders/surface_y_xuxv.frag.qsb",
        GL_TEXTURE_2D, 2, false,
        QSGMaterial::Blending,
        {}
    }
};

WaylandBufferMaterialShader::WaylandBufferMaterialShader(WaylandBufferRef::BufferFormatEgl format)
{
    Q_UNUSED(format);
    setShaderFileName(VertexStage, QString::fromLatin1(bufferTypes[format].vertexShaderSourceFile));
    auto fragmentShaderSourceFile = QString::fromLatin1(bufferTypes[format].fragmentShaderSourceFile);

    if (format == WaylandBufferRef::BufferFormatEgl_EXTERNAL_OES)
        setupExternalOESShader(fragmentShaderSourceFile);
    else
        setShaderFileName(FragmentStage, fragmentShaderSourceFile);
}

void WaylandBufferMaterialShader::setupExternalOESShader(const QString &shaderFilename)
{
#if QT_CONFIG(opengl)
    QFile shaderFile(shaderFilename);
    if (!shaderFile.open(QIODevice::ReadOnly)) {
        qCWarning(qLcWaylandCompositor) << "Cannot find external OES shader file:" << shaderFilename;
        return;
    }
    QByteArray FS = shaderFile.readAll();

    static const char *FS_GLES_PREAMBLE =
        "#extension GL_OES_EGL_image_external : require\n"
        "precision highp float;\n";
    static const char *FS_GL_PREAMBLE =
            "#version 120\n"
            "#extension GL_OES_EGL_image_external : require\n";
    QByteArray fsGLES = FS_GLES_PREAMBLE + FS;
    QByteArray fsGL = FS_GL_PREAMBLE + FS;

    QShaderDescription desc;
    QShaderDescriptionPrivate *descData = QShaderDescriptionPrivate::get(&desc);

    QShaderDescription::InOutVariable texCoordInput;
    texCoordInput.name = "v_texcoord";
    texCoordInput.type = QShaderDescription::Vec2;
    texCoordInput.location = 0;

    descData->inVars = { texCoordInput };

    QShaderDescription::InOutVariable fragColorOutput;
    texCoordInput.name = "gl_FragColor";
    texCoordInput.type = QShaderDescription::Vec4;
    texCoordInput.location = 0;

    descData->outVars = { fragColorOutput };

    QShaderDescription::BlockVariable matrixBlockVar;
    matrixBlockVar.name = "qt_Matrix";
    matrixBlockVar.type = QShaderDescription::Mat4;
    matrixBlockVar.offset = 0;
    matrixBlockVar.size = 64;

    QShaderDescription::BlockVariable opacityBlockVar;
    opacityBlockVar.name = "qt_Opacity";
    opacityBlockVar.type = QShaderDescription::Float;
    opacityBlockVar.offset = 64;
    opacityBlockVar.size = 4;

    QShaderDescription::UniformBlock ubufStruct;
    ubufStruct.blockName = "buf";
    ubufStruct.structName = "ubuf";
    ubufStruct.size = 64 + 4;
    ubufStruct.binding = 0;
    ubufStruct.members = { matrixBlockVar, opacityBlockVar };

    descData->uniformBlocks = { ubufStruct };

    QShaderDescription::InOutVariable samplerTex0;
    samplerTex0.name = "tex0";
    samplerTex0.type = QShaderDescription::SamplerExternalOES;
    samplerTex0.binding = 1;

    descData->combinedImageSamplers = { samplerTex0 };

    QShader shaderPack;
    shaderPack.setStage(QShader::FragmentStage);
    shaderPack.setDescription(desc);
    shaderPack.setShader(QShaderKey(QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs)), QShaderCode(fsGLES));
    shaderPack.setShader(QShaderKey(QShader::GlslShader, QShaderVersion(120)), QShaderCode(fsGL));

    setShader(FragmentStage, shaderPack);
#else
    Q_UNUSED(shaderFilename);
#endif
}

bool WaylandBufferMaterialShader::updateUniformData(RenderState &state, QSGMaterial *, QSGMaterial *)
{
    bool changed = false;
    QByteArray *buf = state.uniformData();
    Q_ASSERT(buf->size() >= 68);

    if (state.isMatrixDirty()) {
        const QMatrix4x4 m = state.combinedMatrix();
        memcpy(buf->data(), m.constData(), 64);
        changed = true;
    }

    if (state.isOpacityDirty()) {
        const float opacity = state.opacity();
        memcpy(buf->data() + 64, &opacity, 4);
        changed = true;
    }

    return changed;
}

void WaylandBufferMaterialShader::updateSampledImage(RenderState &state, int binding, QSGTexture **texture,
                                                      QSGMaterial *newMaterial, QSGMaterial *)
{
    Q_UNUSED(state);

    WaylandBufferMaterial *material = static_cast<WaylandBufferMaterial *>(newMaterial);
    switch (binding) {
    case 1:
        *texture = material->m_scenegraphTextures.at(0);
        break;
    case 2:
        *texture = material->m_scenegraphTextures.at(1);
        break;
    case 3:
        *texture = material->m_scenegraphTextures.at(2);
        break;
    default:
        return;
    }

    // This is for the shared memory case, and is a no-op for others,
    // this is where the upload from the QImage happens when not yet done.
    // ### or is this too late? (if buffer.image() disappears in the meantime then this is the wrong...)
    if (*texture)
        (*texture)->commitTextureOperations(state.rhi(), state.resourceUpdateBatch());
}

WaylandBufferMaterial::WaylandBufferMaterial(WaylandBufferRef::BufferFormatEgl format)
    : m_format(format)
{
    setFlag(bufferTypes[m_format].materialFlags);
}

WaylandBufferMaterial::~WaylandBufferMaterial()
{
    qDeleteAll(m_scenegraphTextures);
}

void WaylandBufferMaterial::setTextureForPlane(int plane,
                                                QOpenGLTexture *texture,
                                                QSGTexture *scenegraphTexture)
{
    if (plane < 0 || plane >= bufferTypes[m_format].planeCount) {
        qWarning("plane index is out of range");
        return;
    }

    texture->bind();
    setTextureParameters(texture->target());

    ensureTextures(plane - 1);

    if (m_textures.size() <= plane) {
        m_textures << texture;
        m_scenegraphTextures << scenegraphTexture;
    } else {
        delete m_scenegraphTextures[plane];

        m_textures[plane] = texture;
        m_scenegraphTextures[plane] = scenegraphTexture;
    }
}

void WaylandBufferMaterial::bind()
{
    ensureTextures(bufferTypes[m_format].planeCount);

    switch (m_textures.size()) {
    case 3:
        if (m_textures[2])
            m_textures[2]->bind(2);
        Q_FALLTHROUGH();
    case 2:
        if (m_textures[1])
            m_textures[1]->bind(1);
        Q_FALLTHROUGH();
    case 1:
        if (m_textures[0])
            m_textures[0]->bind(0);
    }
}

QSGMaterialType *WaylandBufferMaterial::type() const
{
    return const_cast<QSGMaterialType *>(&bufferTypes[m_format].materialType);
}

QSGMaterialShader *WaylandBufferMaterial::createShader(QSGRendererInterface::RenderMode renderMode) const
{
    Q_UNUSED(renderMode);
    return new WaylandBufferMaterialShader(m_format);
}


void WaylandBufferMaterial::setTextureParameters(GLenum target)
{
    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();
    gl->glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

//TODO move this into a separate centralized texture management class
void WaylandBufferMaterial::ensureTextures(int count)
{
    for (int plane = m_textures.size(); plane < count; plane++) {
        m_textures << nullptr;
        m_scenegraphTextures << nullptr;
    }
}

void WaylandBufferMaterial::setBufferRef(WaylandQuickItem *surfaceItem, const WaylandBufferRef &ref)
{
    m_bufferRef = ref;
    for (int plane = 0; plane < bufferTypes[ref.bufferFormatEgl()].planeCount; plane++) {
        if (auto texture = ref.toOpenGLTexture(plane)) {
            QQuickWindow::CreateTextureOptions opt;
            WaylandQuickSurface *waylandSurface = qobject_cast<WaylandQuickSurface *>(surfaceItem->surface());
            if (waylandSurface != nullptr && waylandSurface->useTextureAlpha() && !waylandSurface->isOpaque())
                opt |= QQuickWindow::TextureHasAlphaChannel;
            QSGTexture *scenegraphTexture;
            if (ref.bufferFormatEgl() == WaylandBufferRef::BufferFormatEgl_EXTERNAL_OES) {
                scenegraphTexture = QNativeInterface::QSGOpenGLTexture::fromNativeExternalOES(texture->textureId(),
                                                                                              surfaceItem->window(),
                                                                                              ref.size(),
                                                                                              opt);
            } else {
                scenegraphTexture = QNativeInterface::QSGOpenGLTexture::fromNative(texture->textureId(),
                                                                                   surfaceItem->window(),
                                                                                   ref.size(),
                                                                                   opt);
            }
            scenegraphTexture->setFiltering(surfaceItem->smooth() ? QSGTexture::Linear : QSGTexture::Nearest);
            setTextureForPlane(plane, texture, scenegraphTexture);
        }
    }

    bind();
}
#else
static const struct {
    const char * const vertexShaderSourceFile;
    const char * const fragmentShaderSourceFile;
    GLenum textureTarget;
    int planeCount;
    bool canProvideTexture;
    QSGMaterial::Flags materialFlags;
    QSGMaterialType materialType;
} bufferTypes[] = {
    // BufferFormatEgl_Null
    { "", "", 0, 0, false, {}, {} },

    // BufferFormatEgl_RGB
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert",
        ":/qt-project.org/wayland/compositor/shaders/surface_rgbx.frag",
        GL_TEXTURE_2D, 1, true,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_RGBA
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert",
        ":/qt-project.org/wayland/compositor/shaders/surface_rgba.frag",
        GL_TEXTURE_2D, 1, true,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_EXTERNAL_OES
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert",
        ":/qt-project.org/wayland/compositor/shaders/surface_oes_external.frag",
        GL_TEXTURE_EXTERNAL_OES, 1, false,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_Y_U_V
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert",
        ":/qt-project.org/wayland/compositor/shaders/surface_y_u_v.frag",
        GL_TEXTURE_2D, 3, false,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_Y_UV
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert",
        ":/qt-project.org/wayland/compositor/shaders/surface_y_uv.frag",
        GL_TEXTURE_2D, 2, false,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_Y_XUXV
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert",
        ":/qt-project.org/wayland/compositor/shaders/surface_y_xuxv.frag",
        GL_TEXTURE_2D, 2, false,
        QSGMaterial::Blending,
        {}
    }
};

WaylandBufferMaterialShader::WaylandBufferMaterialShader(WaylandBufferRef::BufferFormatEgl format)
    : m_format(format)
{
    setShaderSourceFile(QOpenGLShader::Vertex, QString::fromLatin1(bufferTypes[format].vertexShaderSourceFile));
    setShaderSourceFile(QOpenGLShader::Fragment, QString::fromLatin1(bufferTypes[format].fragmentShaderSourceFile));
}

void WaylandBufferMaterialShader::updateState(const QSGMaterialShader::RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect)
{
    QSGMaterialShader::updateState(state, newEffect, oldEffect);

    WaylandBufferMaterial *material = static_cast<WaylandBufferMaterial *>(newEffect);
    material->bind();

    if (state.isMatrixDirty())
        program()->setUniformValue(m_id_matrix, state.combinedMatrix());

    if (state.isOpacityDirty())
        program()->setUniformValue(m_id_opacity, state.opacity());
}

const char * const *WaylandBufferMaterialShader::attributeNames() const
{
    static char const *const attr[] = { "qt_VertexPosition", "qt_VertexTexCoord", nullptr };
    return attr;
}

void WaylandBufferMaterialShader::initialize()
{
    QSGMaterialShader::initialize();

    m_id_matrix = program()->uniformLocation("qt_Matrix");
    m_id_opacity = program()->uniformLocation("qt_Opacity");

    for (int i = 0; i < bufferTypes[m_format].planeCount; i++) {
        m_id_tex << program()->uniformLocation("tex" + QByteArray::number(i));
        program()->setUniformValue(m_id_tex[i], i);
    }

    Q_ASSERT(m_id_tex.size() == bufferTypes[m_format].planeCount);
}

WaylandBufferMaterial::WaylandBufferMaterial(WaylandBufferRef::BufferFormatEgl format)
    : m_format(format)
{
    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();

    gl->glBindTexture(bufferTypes[m_format].textureTarget, 0);
    setFlag(bufferTypes[m_format].materialFlags);
}

WaylandBufferMaterial::~WaylandBufferMaterial()
{
}

void WaylandBufferMaterial::setTextureForPlane(int plane, QOpenGLTexture *texture)
{
    if (plane < 0 || plane >= bufferTypes[m_format].planeCount) {
        qWarning("plane index is out of range");
        return;
    }

    texture->bind();
    setTextureParameters(texture->target());

    ensureTextures(plane - 1);

    if (m_textures.size() <= plane)
        m_textures << texture;
    else
        m_textures[plane] = texture;
}

void WaylandBufferMaterial::bind()
{
    ensureTextures(bufferTypes[m_format].planeCount);

    switch (m_textures.size()) {
    case 3:
        if (m_textures[2])
            m_textures[2]->bind(2);
        Q_FALLTHROUGH();
    case 2:
        if (m_textures[1])
            m_textures[1]->bind(1);
        Q_FALLTHROUGH();
    case 1:
        if (m_textures[0])
            m_textures[0]->bind(0);
    }
}

QSGMaterialType *WaylandBufferMaterial::type() const
{
    return const_cast<QSGMaterialType *>(&bufferTypes[m_format].materialType);
}

QSGMaterialShader *WaylandBufferMaterial::createShader() const
{
    return new WaylandBufferMaterialShader(m_format);
}


void WaylandBufferMaterial::setTextureParameters(GLenum target)
{
    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();
    gl->glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

//TODO move this into a separate centralized texture management class
void WaylandBufferMaterial::ensureTextures(int count)
{
    for (int plane = m_textures.size(); plane < count; plane++) {
        m_textures << nullptr;
    }
}
#endif
#endif // QT_CONFIG(opengl)

QMutex *WaylandQuickItemPrivate::mutex = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class WaylandSurfaceTextureProvider : public QSGTextureProvider
{
public:
    WaylandSurfaceTextureProvider()
    {
    }

    ~WaylandSurfaceTextureProvider() override
    {
        delete m_sgTex;
    }

    void setBufferRef(WaylandQuickItem *surfaceItem, const WaylandBufferRef &buffer)
    {
        Q_ASSERT(QThread::currentThread() == thread());
        m_ref = buffer;
        delete m_sgTex;
        m_sgTex = nullptr;
        if (m_ref.hasBuffer()) {
            if (buffer.isSharedMemory()) {
                m_sgTex = surfaceItem->window()->createTextureFromImage(buffer.image());
            } else {
#if QT_CONFIG(opengl)
                QQuickWindow::CreateTextureOptions opt;
                WaylandQuickSurface *surface = qobject_cast<WaylandQuickSurface *>(surfaceItem->surface());
                if (surface && surface->useTextureAlpha()  && !surface->isOpaque()) {
                    opt |= QQuickWindow::TextureHasAlphaChannel;
                }

                auto texture = buffer.toOpenGLTexture();
                GLuint textureId = texture->textureId();
                auto size = surface->bufferSize();
                m_sgTex = QNativeInterface::QSGOpenGLTexture::fromNative(textureId, surfaceItem->window(), size, opt);
#else
                qCWarning(qLcWaylandCompositor) << "Without OpenGL support only shared memory textures are supported";
#endif
            }
        }
        emit textureChanged();
    }

    QSGTexture *texture() const override
    {
        if (m_sgTex)
            m_sgTex->setFiltering(m_smooth ? QSGTexture::Linear : QSGTexture::Nearest);
        return m_sgTex;
    }

    void setSmooth(bool smooth) { m_smooth = smooth; }
private:
    bool m_smooth = false;
    QSGTexture *m_sgTex = nullptr;
    WaylandBufferRef m_ref;
};
#else
class WaylandSurfaceTextureProvider : public QSGTextureProvider
{
public:
    WaylandSurfaceTextureProvider()
    {
    }

    ~WaylandSurfaceTextureProvider() override
    {
        if (m_sgTex)
            m_sgTex->deleteLater();
    }

    void setBufferRef(WaylandQuickItem *surfaceItem, const WaylandBufferRef &buffer)
    {
        Q_ASSERT(QThread::currentThread() == thread());
        m_ref = buffer;
        delete m_sgTex;
        m_sgTex = nullptr;
        if (m_ref.hasBuffer()) {
            if (buffer.isSharedMemory()) {
                m_sgTex = surfaceItem->window()->createTextureFromImage(buffer.image());
#if QT_CONFIG(opengl)
                if (m_sgTex)
                    m_sgTex->bind();
#endif
            } else {
#if QT_CONFIG(opengl)
                QQuickWindow::CreateTextureOptions opt;
                WaylandQuickSurface *surface = qobject_cast<WaylandQuickSurface *>(surfaceItem->surface());
                if (surface && surface->useTextureAlpha()  && !surface->isOpaque()) {
                    opt |= QQuickWindow::TextureHasAlphaChannel;
                }

                auto texture = buffer.toOpenGLTexture();
                GLuint textureId = texture->textureId();
                auto size = surface->bufferSize();
                m_sgTex = surfaceItem->window()->createTextureFromNativeObject(QQuickWindow::NativeObjectTexture, &textureId, 0, size, opt);
#else
                qCWarning(qLcWaylandCompositor) << "Without OpenGL support only shared memory textures are supported";
#endif
            }
        }
        emit textureChanged();
    }

    QSGTexture *texture() const override
    {
        if (m_sgTex)
            m_sgTex->setFiltering(m_smooth ? QSGTexture::Linear : QSGTexture::Nearest);
        return m_sgTex;
    }

    void setSmooth(bool smooth) { m_smooth = smooth; }
private:
    bool m_smooth = false;
    QSGTexture *m_sgTex = nullptr;
    WaylandBufferRef m_ref;
};
#endif

/*!
 * \qmltype WaylandQuickItem
 * \instantiates WaylandQuickItem
 * \inqmlmodule Aurora.Compositor
 * \since 5.8
 * \brief Provides a Qt Quick item that represents a WaylandView.
 *
 * Qt Quick-based Wayland compositors can use this type to display a client's
 * contents on an output device. It passes user input to the
 * client.
 */

/*!
 * \class WaylandQuickItem
 * \inmodule AuroraCompositor
 * \since 5.8
 * \brief The WaylandQuickItem class provides a Qt Quick item representing a WaylandView.
 *
 * When writing a WaylandCompositor in Qt Quick, this class can be used to display a
 * client's contents on an output device and will pass user input to the
 * client.
 */

/*!
 * Constructs a WaylandQuickItem with the given \a parent.
 */
WaylandQuickItem::WaylandQuickItem(QQuickItem *parent)
    : QQuickItem(parent)
    , d_ptr(new WaylandQuickItemPrivate(this))
{
    d_func()->init();
}

/*!
 * Destroy the WaylandQuickItem.
 */
WaylandQuickItem::~WaylandQuickItem()
{
    Q_D(WaylandQuickItem);
    disconnect(this, &QQuickItem::windowChanged, this, &WaylandQuickItem::updateWindow);
    QMutexLocker locker(d->mutex);
    if (d->provider)
        d->provider->deleteLater();
}

/*!
 * \qmlproperty WaylandCompositor AuroraCompositor::WaylandQuickItem::compositor
 *
 * This property holds the compositor for the surface rendered by this WaylandQuickItem.
 */

/*!
 * \property WaylandQuickItem::compositor
 *
 * This property holds the compositor for the surface rendered by this WaylandQuickItem.
 */
WaylandCompositor *WaylandQuickItem::compositor() const
{
    Q_D(const WaylandQuickItem);
    return d->view->surface() ? d->view->surface()->compositor() : nullptr;
}

/*!
 * Returns the view rendered by this WaylandQuickItem.
 */
WaylandView *WaylandQuickItem::view() const
{
    Q_D(const WaylandQuickItem);
    return d->view.data();
}

/*!
 * \qmlproperty WaylandSurface AuroraCompositor::WaylandQuickItem::surface
 *
 * This property holds the surface rendered by this WaylandQuickItem.
 */

/*!
 * \property WaylandQuickItem::surface
 *
 * This property holds the surface rendered by this WaylandQuickItem.
 */

WaylandSurface *WaylandQuickItem::surface() const
{
    Q_D(const WaylandQuickItem);
    return d->view->surface();
}

void WaylandQuickItem::setSurface(WaylandSurface *surface)
{
    Q_D(WaylandQuickItem);
    WaylandSurface *oldSurf = d->view->surface();
    WaylandCompositor *oldComp = d->view->surface() ? d->view->surface()->compositor() : nullptr;
    d->view->setSurface(surface);
    WaylandCompositor *newComp = d->view->surface() ? d->view->surface()->compositor() : nullptr;
    if (oldComp != newComp)
        emit compositorChanged();
    if (oldSurf != surface)
        emit surfaceChanged();
    update();
}

/*!
 * \qmlproperty enum AuroraCompositor::WaylandQuickItem::origin
 *
 * This property holds the origin of the WaylandQuickItem.
 */

/*!
 * \property WaylandQuickItem::origin
 *
 * This property holds the origin of the WaylandQuickItem.
 */
WaylandSurface::Origin WaylandQuickItem::origin() const
{
    Q_D(const WaylandQuickItem);
    return d->origin;
}

bool WaylandQuickItem::isTextureProvider() const
{
    Q_D(const WaylandQuickItem);
    return QQuickItem::isTextureProvider() || d->provider;
}

/*!
 * Returns the texture provider of this WaylandQuickItem.
 */
QSGTextureProvider *WaylandQuickItem::textureProvider() const
{
    Q_D(const WaylandQuickItem);

    if (QQuickItem::isTextureProvider())
        return QQuickItem::textureProvider();

    return d->provider;
}

/*!
 * \internal
 */
void WaylandQuickItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(WaylandQuickItem);
    if (!d->shouldSendInputEvents()) {
        event->ignore();
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (!inputRegionContains(event->position())) {
#else
    if (!inputRegionContains(event->localPos())) {
#endif
        event->ignore();
        return;
    }

    WaylandSeat *seat = compositor()->seatFor(event);

    if (d->focusOnClick)
        takeFocus(seat);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    seat->sendMouseMoveEvent(d->view.data(), mapToSurface(event->position()), event->scenePosition());
    seat->sendMousePressEvent(event->button());
    d->hoverPos = event->position();
#else
    seat->sendMouseMoveEvent(d->view.data(), mapToSurface(event->localPos()), event->windowPos());
    seat->sendMousePressEvent(event->button());
    d->hoverPos = event->localPos();
#endif
}

/*!
 * \internal
 */
void WaylandQuickItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(WaylandQuickItem);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (d->shouldSendInputEvents()) {
        WaylandSeat *seat = compositor()->seatFor(event);
        if (d->isDragging) {
            WaylandQuickOutput *currentOutput = qobject_cast<WaylandQuickOutput *>(view()->output());
            //TODO: also check if dragging onto other outputs
            WaylandQuickItem *targetItem = qobject_cast<WaylandQuickItem *>(currentOutput->pickClickableItem(mapToScene(event->position())));
            WaylandSurface *targetSurface = targetItem ? targetItem->surface() : nullptr;
            if (targetSurface) {
                QPointF position = mapToItem(targetItem, event->position());
                QPointF surfacePosition = targetItem->mapToSurface(position);
                seat->drag()->dragMove(targetSurface, surfacePosition);
            }
        } else {
            seat->sendMouseMoveEvent(d->view.data(), mapToSurface(event->position()), event->scenePosition());
            d->hoverPos = event->position();
        }
    } else {
        emit mouseMove(event->scenePosition());
        event->ignore();
    }
#else
    if (d->shouldSendInputEvents()) {
        WaylandSeat *seat = compositor()->seatFor(event);
        if (d->isDragging) {
            WaylandQuickOutput *currentOutput = qobject_cast<WaylandQuickOutput *>(view()->output());
            //TODO: also check if dragging onto other outputs
            WaylandQuickItem *targetItem = qobject_cast<WaylandQuickItem *>(currentOutput->pickClickableItem(mapToScene(event->localPos())));
            WaylandSurface *targetSurface = targetItem ? targetItem->surface() : nullptr;
            if (targetSurface) {
                QPointF position = mapToItem(targetItem, event->localPos());
                QPointF surfacePosition = targetItem->mapToSurface(position);
                seat->drag()->dragMove(targetSurface, surfacePosition);
            }
        } else {
            seat->sendMouseMoveEvent(d->view.data(), mapToSurface(event->localPos()), event->windowPos());
            d->hoverPos = event->localPos();
        }
    } else {
        emit mouseMove(event->windowPos());
        event->ignore();
    }
#endif
}

/*!
 * \internal
 */
void WaylandQuickItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(WaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        WaylandSeat *seat = compositor()->seatFor(event);
        if (d->isDragging) {
            d->isDragging = false;
            seat->drag()->drop();
        } else {
            seat->sendMouseReleaseEvent(event->button());
        }
    } else {
        emit mouseRelease();
        event->ignore();
    }
}

/*!
 * \internal
 */
void WaylandQuickItem::hoverEnterEvent(QHoverEvent *event)
{
    Q_D(WaylandQuickItem);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (!inputRegionContains(event->position())) {
        event->ignore();
        return;
    }
    if (d->shouldSendInputEvents()) {
        WaylandSeat *seat = compositor()->seatFor(event);
        seat->sendMouseMoveEvent(d->view.data(), event->position(), mapToScene(event->position()));
        d->hoverPos = event->position();
    } else {
        event->ignore();
    }
#else
    if (!inputRegionContains(event->posF())) {
        event->ignore();
        return;
    }
    if (d->shouldSendInputEvents()) {
        WaylandSeat *seat = compositor()->seatFor(event);
        seat->sendMouseMoveEvent(d->view.data(), event->posF(), mapToScene(event->posF()));
        d->hoverPos = event->posF();
    } else {
        event->ignore();
    }
#endif
}

/*!
 * \internal
 */
void WaylandQuickItem::hoverMoveEvent(QHoverEvent *event)
{
    Q_D(WaylandQuickItem);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (surface()) {
        if (!inputRegionContains(event->position())) {
            event->ignore();
            return;
        }
    }
    if (d->shouldSendInputEvents()) {
        WaylandSeat *seat = compositor()->seatFor(event);
        if (event->position() != d->hoverPos) {
            seat->sendMouseMoveEvent(d->view.data(), mapToSurface(event->position()), mapToScene(event->position()));
            d->hoverPos = event->position();
        }
    } else {
        event->ignore();
    }
#else
    if (surface()) {
        if (!inputRegionContains(event->posF())) {
            event->ignore();
            return;
        }
    }
    if (d->shouldSendInputEvents()) {
        WaylandSeat *seat = compositor()->seatFor(event);
        if (event->posF() != d->hoverPos) {
            seat->sendMouseMoveEvent(d->view.data(), mapToSurface(event->posF()), mapToScene(event->posF()));
            d->hoverPos = event->posF();
        }
    } else {
        event->ignore();
    }
#endif
}

/*!
 * \internal
 */
void WaylandQuickItem::hoverLeaveEvent(QHoverEvent *event)
{
    Q_D(WaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        WaylandSeat *seat = compositor()->seatFor(event);
        seat->setMouseFocus(nullptr);
    } else {
        event->ignore();
    }
}

#if QT_CONFIG(wheelevent)
/*!
 * \internal
 */
void WaylandQuickItem::wheelEvent(QWheelEvent *event)
{
    Q_D(WaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        if (!inputRegionContains(event->position())) {
            event->ignore();
            return;
        }

        WaylandSeat *seat = compositor()->seatFor(event);
        // TODO: fix this to send a single event, when diagonal scrolling is supported
        if (event->angleDelta().x() != 0)
            seat->sendMouseWheelEvent(Qt::Horizontal, event->angleDelta().x());
        if (event->angleDelta().y() != 0)
            seat->sendMouseWheelEvent(Qt::Vertical, event->angleDelta().y());
    } else {
        event->ignore();
    }
}
#endif

/*!
 * \internal
 */
void WaylandQuickItem::keyPressEvent(QKeyEvent *event)
{
    Q_D(WaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        WaylandSeat *seat = compositor()->seatFor(event);
        if (seat->setKeyboardFocus(d->view->surface()))
            seat->sendFullKeyEvent(event);
        else
            qWarning() << "Unable to set keyboard focus, cannot send key press event";
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void WaylandQuickItem::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(WaylandQuickItem);
    if (d->shouldSendInputEvents() && hasFocus()) {
        WaylandSeat *seat = compositor()->seatFor(event);
        seat->sendFullKeyEvent(event);
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void WaylandQuickItem::touchEvent(QTouchEvent *event)
{
    Q_D(WaylandQuickItem);
    if (d->shouldSendInputEvents() && d->touchEventsEnabled) {
        WaylandSeat *seat = compositor()->seatFor(event);

        QPointF pointPos;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const QList<QTouchEvent::TouchPoint> &points = event->points();
        if (!points.isEmpty())
            pointPos = points.at(0).position();
#else
        const QList<QTouchEvent::TouchPoint> &points = event->touchPoints();
        if (!points.isEmpty())
            pointPos = points.at(0).pos();
#endif

        if (event->type() == QEvent::TouchBegin && !inputRegionContains(pointPos)) {
            event->ignore();
            return;
        }

        event->accept();
        if (seat->mouseFocus() != d->view.data()) {
            seat->sendMouseMoveEvent(d->view.data(), pointPos, mapToScene(pointPos));
        }
        seat->sendFullTouchEvent(surface(), event);

        if (event->type() == QEvent::TouchBegin) {
            d->touchingSeats.append(seat);
        } else if (event->type() == QEvent::TouchEnd || event->type() == QEvent::TouchCancel) {
            d->touchingSeats.removeOne(seat);
        }

        if (event->type() == QEvent::TouchBegin && d->focusOnClick)
            takeFocus(seat);
    } else {
        event->ignore();
    }
}

void WaylandQuickItem::touchUngrabEvent()
{
    Q_D(WaylandQuickItem);

    if (d->shouldSendInputEvents())
        for (const auto seat : qAsConst(d->touchingSeats))
            seat->sendTouchCancelEvent(surface()->client());

    d->touchingSeats.clear();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
/*!
 * \internal
 */
void WaylandQuickItem::inputMethodEvent(QInputMethodEvent *event)
{
    Q_D(WaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        d->oldSurface->inputMethodControl()->inputMethodEvent(event);
    } else {
        event->ignore();
    }
}
#endif
#endif

/*!
 * \internal
 */
void WaylandQuickItem::surfaceChangedEvent(WaylandSurface *newSurface, WaylandSurface *oldSurface)
{
    Q_UNUSED(newSurface);
    Q_UNUSED(oldSurface);
}

void WaylandQuickItem::handleSubsurfaceAdded(WaylandSurface *childSurface)
{
    Q_D(WaylandQuickItem);
    if (d->subsurfaceHandler.isNull()) {
        WaylandQuickItem *childItem = new WaylandQuickItem;
        childItem->setSurface(childSurface);
        childItem->setVisible(true);
        childItem->setParentItem(this);
        childItem->setParent(this);
        connect(childSurface, &WaylandSurface::subsurfacePositionChanged, childItem, &WaylandQuickItem::handleSubsurfacePosition);
        connect(childSurface, &WaylandSurface::destroyed, childItem, &QObject::deleteLater);
    } else {
        bool success = QMetaObject::invokeMethod(d->subsurfaceHandler, "handleSubsurfaceAdded", Q_ARG(WaylandSurface*, childSurface));
        if (!success)
            success = QMetaObject::invokeMethod(d->subsurfaceHandler, "handleSubsurfaceAdded",
                                                Q_ARG(QVariant, QVariant::fromValue(childSurface)));

        if (!success)
            qWarning("WaylandQuickItem: subsurfaceHandler does not implement handleSubsurfaceAdded()");
    }
}

void WaylandQuickItem::handlePlaceAbove(WaylandSurface *referenceSurface)
{
    Q_D(WaylandQuickItem);
    auto *parent = qobject_cast<WaylandQuickItem*>(parentItem());
    if (!parent)
        return;

    if (parent->surface() == referenceSurface) {
        d->placeAboveParent();
    } else if (auto *sibling = d->findSibling(referenceSurface)) {
        d->placeAboveSibling(sibling);
    } else {
        qWarning() << "Couldn't find WaylandQuickItem for surface" << referenceSurface
                   << "when handling wl_subsurface.place_above";
    }
}

void WaylandQuickItem::handlePlaceBelow(WaylandSurface *referenceSurface)
{
    Q_D(WaylandQuickItem);
    WaylandQuickItem *parent = qobject_cast<WaylandQuickItem*>(parentItem());
    if (!parent)
        return;

    if (parent->surface() == referenceSurface) {
        d->placeBelowParent();
    } else if (auto *sibling = d->findSibling(referenceSurface)) {
        d->placeBelowSibling(sibling);
    } else {
        qWarning() << "Couldn't find WaylandQuickItem for surface" << referenceSurface
                   << "when handling wl_subsurface.place_below";
    }
}

/*!
  \qmlproperty object AuroraCompositor::WaylandQuickItem::subsurfaceHandler

  This property provides a way to override the default subsurface behavior.

  By default, Qt will create a new SurfaceItem as a child of this item, and maintain the correct position.

  To override the default, assign a handler object to this property. The handler should implement
  a handleSubsurfaceAdded(WaylandSurface) function.

  \code
  ShellSurfaceItem {
      subsurfaceHandler: QtObject {
          function handleSubsurfaceAdded(child) {
            // create custom surface item, and connect the subsurfacePositionChanged signal
          }
      }
  }
  \endcode

  The default value of this property is \c null.
 */


QObject *WaylandQuickItem::subsurfaceHandler() const
{
    Q_D(const WaylandQuickItem);
    return d->subsurfaceHandler.data();
}

void WaylandQuickItem::setSubsurfaceHandler(QObject *handler)
{
    Q_D(WaylandQuickItem);
    if (d->subsurfaceHandler.data() != handler) {
        d->subsurfaceHandler = handler;
        emit subsurfaceHandlerChanged();
    }
}

/*!
 * \property WaylandQuickItem::output
 *
 * This property holds the output on which this item is displayed.
 */
WaylandOutput *WaylandQuickItem::output() const
{
    Q_D(const WaylandQuickItem);
    return d->view->output();
}

void WaylandQuickItem::setOutput(WaylandOutput *output)
{
    Q_D(WaylandQuickItem);
    d->view->setOutput(output);
}

/*!
 * \qmlproperty bool AuroraCompositor::WaylandQuickItem::bufferLocked
 *
 * This property holds whether the item's buffer is currently locked. As long as
 * the buffer is locked, it will not be released and returned to the client.
 *
 * The default is false.
 */
/*!
 * \property WaylandQuickItem::bufferLocked
 *
 * This property holds whether the item's buffer is currently locked. As long as
 * the buffer is locked, it will not be released and returned to the client.
 *
 * The default is false.
 */
bool WaylandQuickItem::isBufferLocked() const
{
    Q_D(const WaylandQuickItem);
    return d->view->isBufferLocked();
}

void WaylandQuickItem::setBufferLocked(bool locked)
{
    Q_D(WaylandQuickItem);
    d->view->setBufferLocked(locked);

    // Apply the latest surface size
    if (!locked)
        updateSize();
}

/*!
 * \property WaylandQuickItem::allowDiscardFrontBuffer
 *
 * By default, the item locks the current buffer until a new buffer is available
 * and updatePaintNode() is called. Set this property to true to allow Qt to release the buffer
 * immediately when the throttling view is no longer using it. This is useful for items that have
 * slow update intervals.
 */
bool WaylandQuickItem::allowDiscardFrontBuffer() const
{
    Q_D(const WaylandQuickItem);
    return d->view->allowDiscardFrontBuffer();
}

void WaylandQuickItem::setAllowDiscardFrontBuffer(bool discard)
{
    Q_D(WaylandQuickItem);
    d->view->setAllowDiscardFrontBuffer(discard);
}

/*!
 * \qmlmethod WaylandQuickItem::setPrimary()
 *
 * Makes this WaylandQuickItem the primary view for the surface.
 */

/*!
 * Makes this WaylandQuickItem's view the primary view for the surface.
 *
 * \sa WaylandSurface::primaryView
 */
void WaylandQuickItem::setPrimary()
{
    Q_D(WaylandQuickItem);
    d->view->setPrimary();
}

/*!
 * \internal
 */
void WaylandQuickItem::handleSurfaceChanged()
{
    Q_D(WaylandQuickItem);
    if (d->oldSurface) {
        disconnect(d->oldSurface.data(), &WaylandSurface::hasContentChanged, this, &WaylandQuickItem::surfaceMappedChanged);
        disconnect(d->oldSurface.data(), &WaylandSurface::parentChanged, this, &WaylandQuickItem::parentChanged);
        disconnect(d->oldSurface.data(), &WaylandSurface::destinationSizeChanged, this, &WaylandQuickItem::updateSize);
        disconnect(d->oldSurface.data(), &WaylandSurface::bufferScaleChanged, this, &WaylandQuickItem::updateSize);
        disconnect(d->oldSurface.data(), &WaylandSurface::configure, this, &WaylandQuickItem::updateBuffer);
        disconnect(d->oldSurface.data(), &WaylandSurface::redraw, this, &QQuickItem::update);
        disconnect(d->oldSurface.data(), &WaylandSurface::childAdded, this, &WaylandQuickItem::handleSubsurfaceAdded);
        disconnect(d->oldSurface.data(), &WaylandSurface::subsurfacePlaceAbove, this, &WaylandQuickItem::handlePlaceAbove);
        disconnect(d->oldSurface.data(), &WaylandSurface::subsurfacePlaceBelow, this, &WaylandQuickItem::handlePlaceBelow);
        disconnect(d->oldSurface.data(), &WaylandSurface::dragStarted, this, &WaylandQuickItem::handleDragStarted);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
        disconnect(d->oldSurface->inputMethodControl(), &WaylandInputMethodControl::updateInputMethod, this, &WaylandQuickItem::updateInputMethod);
#endif
#endif
    }
    if (WaylandSurface *newSurface = d->view->surface()) {
        connect(newSurface, &WaylandSurface::hasContentChanged, this, &WaylandQuickItem::surfaceMappedChanged);
        connect(newSurface, &WaylandSurface::parentChanged, this, &WaylandQuickItem::parentChanged);
        connect(newSurface, &WaylandSurface::destinationSizeChanged, this, &WaylandQuickItem::updateSize);
        connect(newSurface, &WaylandSurface::bufferScaleChanged, this, &WaylandQuickItem::updateSize);
        connect(newSurface, &WaylandSurface::configure, this, &WaylandQuickItem::updateBuffer);
        connect(newSurface, &WaylandSurface::redraw, this, &QQuickItem::update);
        connect(newSurface, &WaylandSurface::childAdded, this, &WaylandQuickItem::handleSubsurfaceAdded);
        connect(newSurface, &WaylandSurface::subsurfacePlaceAbove, this, &WaylandQuickItem::handlePlaceAbove);
        connect(newSurface, &WaylandSurface::subsurfacePlaceBelow, this, &WaylandQuickItem::handlePlaceBelow);
        connect(newSurface, &WaylandSurface::dragStarted, this, &WaylandQuickItem::handleDragStarted);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
        connect(newSurface->inputMethodControl(), &WaylandInputMethodControl::updateInputMethod, this, &WaylandQuickItem::updateInputMethod);
#endif
#endif

        if (newSurface->origin() != d->origin) {
            d->origin = newSurface->origin();
            emit originChanged();
        }
        if (window()) {
            WaylandOutput *output = newSurface->compositor()->outputFor(window());
            d->view->setOutput(output);
        }
        for (auto &subsurface : WaylandSurfacePrivate::get(newSurface)->subsurfaceChildren) {
            if (!subsurface.isNull())
                handleSubsurfaceAdded(subsurface.data());
        }

        updateSize();
    }
    surfaceChangedEvent(d->view->surface(), d->oldSurface);
    d->oldSurface = d->view->surface();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
    updateInputMethod(Qt::ImQueryInput);
#endif
#endif
}

/*!
 * Calling this function causes the item to take the focus of the
 * input \a device.
 */
void WaylandQuickItem::takeFocus(WaylandSeat *device)
{
    forceActiveFocus();

    if (!surface() || !surface()->client())
        return;

    WaylandSeat *target = device;
    if (!target) {
        target = compositor()->defaultSeat();
    }
    target->setKeyboardFocus(surface());

    qCDebug(qLcWaylandCompositorInputMethods) << Q_FUNC_INFO << " surface:" << surface()
        << ", client:" << surface()->client()
        << ", textinputprotocol:" << (int)(surface()->client()->textInputProtocols());
    if (surface()->client()->textInputProtocols().testFlag(WaylandClient::TextInputProtocol::TextInputV2)) {
        WaylandTextInput *textInput = WaylandTextInput::findIn(target);
        if (textInput)
            textInput->setFocus(surface());
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_WAYLAND_TEXT_INPUT_V4_WIP
    if (surface()->client()->textInputProtocols().testFlag(WaylandClient::TextInputProtocol::TextInputV4)) {
        WaylandTextInputV4 *textInputV4 = WaylandTextInputV4::findIn(target);
        if (textInputV4)
            textInputV4->setFocus(surface());
    }
#endif // QT_WAYLAND_TEXT_INPUT_V4_WIP

    if (surface()->client()->textInputProtocols().testFlag(WaylandClient::TextInputProtocol::QtTextInputMethodV1)) {
        WaylandQtTextInputMethod *textInputMethod = WaylandQtTextInputMethod::findIn(target);
        if (textInputMethod)
            textInputMethod->setFocus(surface());
    }
#endif
}

/*!
 * \internal
 */
void WaylandQuickItem::surfaceMappedChanged()
{
    update();
}

/*!
 * \internal
 */
void WaylandQuickItem::parentChanged(WaylandSurface *newParent, WaylandSurface *oldParent)
{
    Q_UNUSED(oldParent);

    if (newParent) {
        setPaintEnabled(true);
        setVisible(true);
        setOpacity(1);
        setEnabled(true);
    }
}

/*!
 * \internal
 */
void WaylandQuickItem::updateSize()
{
    Q_D(WaylandQuickItem);

    // No resize if buffer is locked
    if (isBufferLocked()) {
        qWarning() << "No update on item size as the buffer is currently locked";
        return;
    }

    QSize size(0, 0);
    if (surface())
        size = surface()->destinationSize() * d->scaleFactor();

    setImplicitSize(size.width(), size.height());
}

/*!
 * \qmlproperty bool AuroraCompositor::WaylandQuickItem::focusOnClick
 *
 * This property specifies whether the WaylandQuickItem should take focus when
 * it is clicked or touched.
 *
 * The default is \c true.
 */

/*!
 * \property WaylandQuickItem::focusOnClick
 *
 * This property specifies whether the WaylandQuickItem should take focus when
 * it is clicked or touched.
 *
 * The default is \c true.
 */
bool WaylandQuickItem::focusOnClick() const
{
    Q_D(const WaylandQuickItem);
    return d->focusOnClick;
}

void WaylandQuickItem::setFocusOnClick(bool focus)
{
    Q_D(WaylandQuickItem);
    if (d->focusOnClick == focus)
        return;

    d->focusOnClick = focus;
    emit focusOnClickChanged();
}

/*!
 * Returns \c true if the input region of this item's surface contains the
 * position given by \a localPosition.
 */
bool WaylandQuickItem::inputRegionContains(const QPointF &localPosition) const
{
    if (WaylandSurface *s = surface())
        return s->inputRegionContains(mapToSurface(localPosition));
    return false;
}

/*!
 * \qmlmethod point WaylandQuickItem::mapToSurface(point point)
 *
 * Maps the given \a point in this item's coordinate system to the equivalent
 * point within the Wayland surface's coordinate system, and returns the mapped
 * coordinate.
 */

/*!
 * Maps the given \a point in this item's coordinate system to the equivalent
 * point within the Wayland surface's coordinate system, and returns the mapped
 * coordinate.
 */
QPointF WaylandQuickItem::mapToSurface(const QPointF &point) const
{
    Q_D(const WaylandQuickItem);
    if (!surface() || surface()->destinationSize().isEmpty())
        return point / d->scaleFactor();

    qreal xScale = width() / surface()->destinationSize().width();
    qreal yScale = height() / surface()->destinationSize().height();

    return QPointF(point.x() / xScale, point.y() / yScale);
}

/*!
 * \qmlmethod point WaylandQuickItem::mapFromSurface(point point)
 * \since 5.13
 *
 * Maps the given \a point in the Wayland surfaces's coordinate system to the equivalent
 * point within this item's coordinate system, and returns the mapped coordinate.
 */

/*!
 * Maps the given \a point in the Wayland surfaces's coordinate system to the equivalent
 * point within this item's coordinate system, and returns the mapped coordinate.
 *
 * \since 5.13
 */
QPointF WaylandQuickItem::mapFromSurface(const QPointF &point) const
{
    Q_D(const WaylandQuickItem);
    if (!surface() || surface()->destinationSize().isEmpty())
        return point * d->scaleFactor();

    qreal xScale = width() / surface()->destinationSize().width();
    qreal yScale = height() / surface()->destinationSize().height();

    return QPointF(point.x() * xScale, point.y() * yScale);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
QVariant WaylandQuickItem::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return inputMethodQuery(query, QVariant());
}

QVariant WaylandQuickItem::inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const
{
    Q_D(const WaylandQuickItem);

    if (query == Qt::ImEnabled)
        return QVariant((flags() & ItemAcceptsInputMethod) != 0);

    if (d->oldSurface)
        return d->oldSurface->inputMethodControl()->inputMethodQuery(query, argument);

    return QVariant();
}
#endif
#endif

/*!
    \qmlproperty bool AuroraCompositor::WaylandQuickItem::paintEnabled

    Returns true if the item is hidden, though the texture
    is still updated. As opposed to hiding the item by
    setting \l{Item::visible}{visible} to \c false, setting this property to \c false
    will not prevent mouse or keyboard input from reaching item.
*/

/*!
    \property WaylandQuickItem::paintEnabled

    Holds \c true if the item is hidden, though the texture
    is still updated. As opposed to hiding the item by
    setting \l{QQuickItem::}{visible} to \c false, setting this property to \c false
    will not prevent mouse or keyboard input from reaching item.
*/
bool WaylandQuickItem::isPaintEnabled() const
{
    Q_D(const WaylandQuickItem);
    return d->paintEnabled;
}

void WaylandQuickItem::setPaintEnabled(bool enabled)
{
    Q_D(WaylandQuickItem);

    if (enabled != d->paintEnabled) {
        d->paintEnabled = enabled;
        emit paintEnabledChanged();
    }

    update();
}

/*!
    \qmlproperty  bool AuroraCompositor::WaylandQuickItem::touchEventsEnabled

    This property holds \c true if touch events are forwarded to the client
    surface, \c false otherwise.
*/

/*!
    \property WaylandQuickItem::touchEventsEnabled

    This property holds \c true if touch events are forwarded to the client
    surface, \c false otherwise.
*/
bool WaylandQuickItem::touchEventsEnabled() const
{
    Q_D(const WaylandQuickItem);
    return d->touchEventsEnabled;
}

void WaylandQuickItem::updateBuffer(bool hasBuffer)
{
    Q_D(WaylandQuickItem);
    Q_UNUSED(hasBuffer);
    if (d->origin != surface()->origin()) {
        d->origin = surface()->origin();
        emit originChanged();
    }
}

void WaylandQuickItem::updateWindow()
{
    Q_D(WaylandQuickItem);

    QQuickWindow *newWindow = window();
    if (newWindow == d->connectedWindow)
        return;

    if (d->connectedWindow) {
        disconnect(d->connectedWindow, &QQuickWindow::beforeSynchronizing, this, &WaylandQuickItem::beforeSync);
        disconnect(d->connectedWindow, &QQuickWindow::screenChanged, this, &WaylandQuickItem::updateSize);
    }

    d->connectedWindow = newWindow;

    if (d->connectedWindow) {
        connect(d->connectedWindow, &QQuickWindow::beforeSynchronizing, this, &WaylandQuickItem::beforeSync, Qt::DirectConnection);
        connect(d->connectedWindow, &QQuickWindow::screenChanged, this, &WaylandQuickItem::updateSize); // new screen may have new dpr

        if (compositor()) {
            WaylandOutput *output = compositor()->outputFor(d->connectedWindow);
            Q_ASSERT(output);
            d->view->setOutput(output);
        }

        updateSize(); // because scaleFactor depends on devicePixelRatio, which may be different for the new window
    }
}

void WaylandQuickItem::updateOutput()
{
    Q_D(WaylandQuickItem);
    if (d->view->output() == d->connectedOutput)
        return;

    if (d->connectedOutput)
        disconnect(d->connectedOutput, &WaylandOutput::scaleFactorChanged, this, &WaylandQuickItem::updateSize);

    d->connectedOutput = d->view->output();

    if (d->connectedOutput)
        connect(d->connectedOutput, &WaylandOutput::scaleFactorChanged, this, &WaylandQuickItem::updateSize);

    updateSize();
}

void WaylandQuickItem::beforeSync()
{
    Q_D(WaylandQuickItem);
    if (d->view->advance()) {
        d->newTexture = true;
        update();
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(im)
void WaylandQuickItem::updateInputMethod(Qt::InputMethodQueries queries)
{
    Q_D(WaylandQuickItem);

    setFlag(QQuickItem::ItemAcceptsInputMethod,
            d->oldSurface ? d->oldSurface->inputMethodControl()->enabled() : false);
    QQuickItem::updateInputMethod(queries | Qt::ImEnabled);
}
#endif
#endif

/*!
 * \qmlsignal void AuroraCompositor::WaylandQuickItem::surfaceDestroyed()
 *
 * This signal is emitted when the client has destroyed the \c wl_surface associated
 * with the WaylandQuickItem. The handler for this signal is expected to either destroy the
 * WaylandQuickItem immediately or start a close animation and then destroy the Item.
 *
 * If an animation is started, bufferLocked should be set to ensure the item keeps its content
 * until the animation finishes
 *
 * \sa bufferLocked
 */

/*!
 * \fn void WaylandQuickItem::surfaceDestroyed()
 *
 * This signal is emitted when the client has destroyed the \c wl_surface associated
 * with the WaylandQuickItem. The handler for this signal is expected to either destroy the
 * WaylandQuickItem immediately or start a close animation and then destroy the Item.
 *
 * If an animation is started, bufferLocked should be set to ensure the item keeps its content
 * until the animation finishes
 *
 * \sa WaylandQuickItem::bufferLocked
 */

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QSGNode *WaylandQuickItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_D(WaylandQuickItem);
    d->lastMatrix = data->transformNode->combinedMatrix();
    const bool bufferHasContent = d->view->currentBuffer().hasContent();

    if (d->view->isBufferLocked() && d->paintEnabled)
        return oldNode;

    if (!bufferHasContent || !d->paintEnabled || !surface()) {
        delete oldNode;
        return nullptr;
    }

    WaylandBufferRef ref = d->view->currentBuffer();
    const bool invertY = ref.origin() == WaylandSurface::OriginBottomLeft;
    const QRectF rect = invertY ? QRectF(0, height(), width(), -height())
                                : QRectF(0, 0, width(), height());

    if (ref.isSharedMemory()
#if QT_CONFIG(opengl)
            || bufferTypes[ref.bufferFormatEgl()].canProvideTexture
#endif
    ) {
#if QT_CONFIG(opengl)
        if (oldNode && !d->paintByProvider) {
            // Need to re-create a node
            delete oldNode;
            oldNode = nullptr;
        }
        d->paintByProvider = true;
#endif
        // This case could covered by the more general path below, but this is more efficient (especially when using ShaderEffect items).
        QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);

        if (!node) {
            node = new QSGSimpleTextureNode();
            if (smooth())
                node->setFiltering(QSGTexture::Linear);
            d->newTexture = true;
        }

        if (!d->provider)
            d->provider = new WaylandSurfaceTextureProvider();

        if (d->newTexture) {
            d->newTexture = false;
            d->provider->setBufferRef(this, ref);
            node->setTexture(d->provider->texture());
        }

        d->provider->setSmooth(smooth());
        node->setRect(rect);

        qreal scale = surface()->bufferScale();
        QRectF source = surface()->sourceGeometry();
        node->setSourceRect(QRectF(source.topLeft() * scale, source.size() * scale));

        return node;
    }

#if QT_CONFIG(opengl)
    Q_ASSERT(!d->provider);

    if (oldNode && d->paintByProvider) {
        // Need to re-create a node
        delete oldNode;
        oldNode = nullptr;
    }
    d->paintByProvider = false;

    QSGGeometryNode *node = static_cast<QSGGeometryNode *>(oldNode);

    if (!node) {
        node = new QSGGeometryNode;
        d->newTexture = true;
    }

    QSGGeometry *geometry = node->geometry();
    WaylandBufferMaterial *material = static_cast<WaylandBufferMaterial *>(node->material());

    if (!geometry)
        geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);

    if (!material)
        material = new WaylandBufferMaterial(ref.bufferFormatEgl());

    if (d->newTexture) {
        d->newTexture = false;
        material->setBufferRef(this, ref);
    }

    const QSize surfaceSize = ref.size() / surface()->bufferScale();
    const QRectF sourceGeometry = surface()->sourceGeometry();
    const QRectF normalizedCoordinates =
            sourceGeometry.isValid()
            ? QRectF(sourceGeometry.x() / surfaceSize.width(),
                     sourceGeometry.y() / surfaceSize.height(),
                     sourceGeometry.width() / surfaceSize.width(),
                     sourceGeometry.height() / surfaceSize.height())
            : QRectF(0, 0, 1, 1);

    QSGGeometry::updateTexturedRectGeometry(geometry, rect, normalizedCoordinates);

    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry, true);

    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial, true);

    return node;
#else
    qCWarning(qLcWaylandCompositor) << "Without OpenGL support only shared memory textures are supported";
    return nullptr;
#endif // QT_CONFIG(opengl)
}
#else
QSGNode *WaylandQuickItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_D(WaylandQuickItem);
    d->lastMatrix = data->transformNode->combinedMatrix();
    const bool bufferHasContent = d->view->currentBuffer().hasContent();

    if (d->view->isBufferLocked() && !bufferHasContent && d->paintEnabled)
        return oldNode;

    if (!bufferHasContent || !d->paintEnabled || !surface()) {
        delete oldNode;
        return nullptr;
    }

    WaylandBufferRef ref = d->view->currentBuffer();
    const bool invertY = ref.origin() == WaylandSurface::OriginBottomLeft;
    const QRectF rect = invertY ? QRectF(0, height(), width(), -height())
                                : QRectF(0, 0, width(), height());

    if (ref.isSharedMemory()
#if QT_CONFIG(opengl)
            || bufferTypes[ref.bufferFormatEgl()].canProvideTexture
#endif
    ) {
        // This case could covered by the more general path below, but this is more efficient (especially when using ShaderEffect items).
        QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);

        if (!node) {
            node = new QSGSimpleTextureNode();
            d->newTexture = true;
        }

        if (!d->provider)
            d->provider = new WaylandSurfaceTextureProvider();

        if (d->newTexture) {
            d->newTexture = false;
            d->provider->setBufferRef(this, ref);
            node->setTexture(d->provider->texture());
        }

        d->provider->setSmooth(smooth());
        node->setRect(rect);

        qreal scale = surface()->bufferScale();
        QRectF source = surface()->sourceGeometry();
        node->setSourceRect(QRectF(source.topLeft() * scale, source.size() * scale));

        return node;
    }

#if QT_CONFIG(opengl)
    Q_ASSERT(!d->provider);

    QSGGeometryNode *node = static_cast<QSGGeometryNode *>(oldNode);

    if (!node) {
        node = new QSGGeometryNode;
        d->newTexture = true;
    }

    QSGGeometry *geometry = node->geometry();
    WaylandBufferMaterial *material = static_cast<WaylandBufferMaterial *>(node->material());

    if (!geometry)
        geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);

    if (!material)
        material = new WaylandBufferMaterial(ref.bufferFormatEgl());

    if (d->newTexture) {
        d->newTexture = false;
        for (int plane = 0; plane < bufferTypes[ref.bufferFormatEgl()].planeCount; plane++)
            if (auto texture = ref.toOpenGLTexture(plane))
                material->setTextureForPlane(plane, texture);
        material->bind();
    }

    QSGGeometry::updateTexturedRectGeometry(geometry, rect, QRectF(0, 0, 1, 1));

    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry, true);

    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial, true);

    return node;
#else
    qCWarning(qLcWaylandCompositor) << "Without OpenGL support only shared memory textures are supported";
    return nullptr;
#endif // QT_CONFIG(opengl)
}
#endif

void WaylandQuickItem::setTouchEventsEnabled(bool enabled)
{
    Q_D(WaylandQuickItem);
    if (d->touchEventsEnabled != enabled) {
        d->touchEventsEnabled = enabled;
        emit touchEventsEnabledChanged();
    }
}

bool WaylandQuickItem::inputEventsEnabled() const
{
    Q_D(const WaylandQuickItem);
    return d->inputEventsEnabled;
}

void WaylandQuickItem::setInputEventsEnabled(bool enabled)
{
    Q_D(WaylandQuickItem);
    if (d->inputEventsEnabled != enabled) {
        if (enabled)
            setEnabled(true);
        d->setInputEventsEnabled(enabled);
        emit inputEventsEnabledChanged();
    }
}

void WaylandQuickItem::lower()
{
    Q_D(WaylandQuickItem);
    d->lower();
}

void WaylandQuickItemPrivate::lower()
{
    Q_Q(WaylandQuickItem);
    QQuickItem *parent = q->parentItem();
    Q_ASSERT(parent);
    QQuickItem *bottom = parent->childItems().constFirst();
    if (q != bottom)
        q->stackBefore(bottom);
}

void WaylandQuickItem::raise()
{
    Q_D(WaylandQuickItem);
    d->raise();
}

void WaylandQuickItemPrivate::raise()
{
    Q_Q(WaylandQuickItem);
    QQuickItem *parent = q->parentItem();
    Q_ASSERT(parent);
    QQuickItem *top = parent->childItems().constLast();
    if (q != top)
        q->stackAfter(top);
}

void WaylandQuickItem::sendMouseMoveEvent(const QPointF &position, WaylandSeat *seat)
{
    if (seat == nullptr)
        seat = compositor()->defaultSeat();

    if (!seat) {
        qWarning() << "No seat, can't send mouse event";
        return;
    }

    seat->sendMouseMoveEvent(view(), position);
}

/*!
 * \internal
 *
 * Sets the position of this item relative to the parent item.
 */
void WaylandQuickItem::handleSubsurfacePosition(const QPoint &pos)
{
    Q_D(WaylandQuickItem);
    QQuickItem::setPosition(pos * d->scaleFactor());
}

void WaylandQuickItem::handleDragStarted(WaylandDrag *drag)
{
    Q_D(WaylandQuickItem);
    Q_ASSERT(drag->origin() == surface());
    drag->seat()->setMouseFocus(nullptr);
    d->isDragging = true;
}

qreal WaylandQuickItemPrivate::scaleFactor() const
{
    Q_Q(const WaylandQuickItem);

    qreal f = view->output() ? view->output()->scaleFactor() : 1;
#if !defined(Q_OS_MACOS)
    if (q->window())
        f /= q->window()->devicePixelRatio();
#endif
    return f;
}

WaylandQuickItem *WaylandQuickItemPrivate::findSibling(WaylandSurface *surface) const
{
    Q_Q(const WaylandQuickItem);
    auto *parent = q->parentItem();
    if (!parent)
        return nullptr;

    const auto siblings = q->parentItem()->childItems();
    for (auto *sibling : siblings) {
        auto *waylandItem = qobject_cast<WaylandQuickItem *>(sibling);
        if (waylandItem && waylandItem->surface() == surface)
            return waylandItem;
    }
    return nullptr;
}

void WaylandQuickItemPrivate::placeAboveSibling(WaylandQuickItem *sibling)
{
    Q_Q(WaylandQuickItem);
    q->stackAfter(sibling);
    q->setZ(sibling->z());
    belowParent = sibling->d_func()->belowParent;
}

void WaylandQuickItemPrivate::placeBelowSibling(WaylandQuickItem *sibling)
{
    Q_Q(WaylandQuickItem);
    q->stackBefore(sibling);
    q->setZ(sibling->z());
    belowParent = sibling->d_func()->belowParent;
}

//### does not handle changes in z value if parent is a subsurface
void WaylandQuickItemPrivate::placeAboveParent()
{
    Q_Q(WaylandQuickItem);
    const auto siblings = q->parentItem()->childItems();

    // Stack below first (bottom) sibling above parent
    bool foundSibling = false;
    for (auto it = siblings.cbegin(); it != siblings.cend(); ++it) {
        WaylandQuickItem *sibling = qobject_cast<WaylandQuickItem*>(*it);
        if (sibling && !sibling->d_func()->belowParent) {
            q->stackBefore(sibling);
            foundSibling = true;
            break;
        }
    }

    // No other subsurfaces above parent
    if (!foundSibling && siblings.last() != q)
        q->stackAfter(siblings.last());

    q->setZ(q->parentItem()->z());
    belowParent = false;
}

//### does not handle changes in z value if parent is a subsurface
void WaylandQuickItemPrivate::placeBelowParent()
{
    Q_Q(WaylandQuickItem);
    const auto siblings = q->parentItem()->childItems();

    // Stack above last (top) sibling below parent
    bool foundSibling = false;
    for (auto it = siblings.crbegin(); it != siblings.crend(); ++it) {
        WaylandQuickItem *sibling = qobject_cast<WaylandQuickItem*>(*it);
        if (sibling && sibling->d_func()->belowParent) {
            q->stackAfter(sibling);
            foundSibling = true;
            break;
        }
    }

    // No other subsurfaces below parent
    if (!foundSibling && siblings.first() != q)
        q->stackBefore(siblings.first());

    q->setZ(q->parentItem()->z() - 1.0);
    belowParent = true;
}

} // namespace Compositor

} // namespace Aurora
