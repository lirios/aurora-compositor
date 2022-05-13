/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#ifndef AURORA_COMPOSITOR_WAYLANDCOMPOSITOR_H
#define AURORA_COMPOSITOR_WAYLANDCOMPOSITOR_H

#include <LiriAuroraCompositor/liriauroracompositorglobal.h>
#include <LiriAuroraCompositor/aurorawaylandqmlinclude.h>
#include <LiriAuroraCompositor/aurorawaylandcompositorextension.h>
#include <LiriAuroraCompositor/WaylandOutput>

#include <QObject>
#include <QImage>
#include <QRect>
#include <QLoggingCategory>

struct wl_display;

class QInputEvent;
class QMimeData;
class QUrl;
class QOpenGLContext;

namespace Aurora {

namespace Compositor {

class WaylandCompositorPrivate;
class WaylandClient;
class WaylandSurface;
class WaylandSeat;
class WaylandView;
class WaylandPointer;
class WaylandKeyboard;
class WaylandTouch;
class WaylandSurfaceGrabber;
class WaylandBufferRef;

LIRIAURORACOMPOSITOR_EXPORT Q_DECLARE_LOGGING_CATEGORY(gLcAuroraCompositor)
LIRIAURORACOMPOSITOR_EXPORT Q_DECLARE_LOGGING_CATEGORY(gLcAuroraCompositorHardwareIntegration)
Q_DECLARE_LOGGING_CATEGORY(gLcAuroraCompositorInputMethods)
Q_DECLARE_LOGGING_CATEGORY(gLcAuroraCompositorTextInput)

class LIRIAURORACOMPOSITOR_EXPORT WaylandCompositor : public WaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandCompositor)
    Q_PROPERTY(QByteArray socketName READ socketName WRITE setSocketName NOTIFY socketNameChanged)
    Q_PROPERTY(bool created READ isCreated NOTIFY createdChanged)
    Q_PROPERTY(bool retainedSelection READ retainedSelectionEnabled WRITE setRetainedSelectionEnabled NOTIFY retainedSelectionChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandOutput *defaultOutput READ defaultOutput WRITE setDefaultOutput NOTIFY defaultOutputChanged)
    Q_PROPERTY(bool useHardwareIntegrationExtension READ useHardwareIntegrationExtension WRITE setUseHardwareIntegrationExtension NOTIFY useHardwareIntegrationExtensionChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandSeat *defaultSeat READ defaultSeat NOTIFY defaultSeatChanged)
    Q_PROPERTY(QVector<ShmFormat> additionalShmFormats READ additionalShmFormats WRITE setAdditionalShmFormats NOTIFY additionalShmFormatsChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    Q_MOC_INCLUDE("aurorawaylandseat.h")
    QML_NAMED_ELEMENT(WaylandCompositorBase)
    QML_UNCREATABLE("Cannot create instance of WaylandCompositorBase, use WaylandCompositor instead")
    QML_ADDED_IN_VERSION(1, 0)
#endif
public:
    // Duplicates subset of supported values wl_shm_format enum
    enum ShmFormat {
        ShmFormat_ARGB8888 = 0,
        ShmFormat_XRGB8888 = 1,
        ShmFormat_C8 = 0x20203843,
        ShmFormat_XRGB4444 = 0x32315258,
        ShmFormat_ARGB4444 = 0x32315241,
        ShmFormat_XRGB1555 = 0x35315258,
        ShmFormat_RGB565 = 0x36314752,
        ShmFormat_RGB888 = 0x34324752,
        ShmFormat_XBGR8888 = 0x34324258,
        ShmFormat_ABGR8888 = 0x34324241,
        ShmFormat_XRGB2101010 = 0x30335258,
        ShmFormat_XBGR2101010 = 0x30334258,
        ShmFormat_ARGB2101010 = 0x30335241,
        ShmFormat_ABGR2101010 = 0x30334241
    };
    Q_ENUM(ShmFormat)

    WaylandCompositor(QObject *parent = nullptr);
    ~WaylandCompositor() override;

    virtual void create();
    bool isCreated() const;

    void setSocketName(const QByteArray &name);
    QByteArray socketName() const;

    Q_INVOKABLE void addSocketDescriptor(int fd);

    ::wl_display *display() const;
    uint32_t nextSerial();

    QList<WaylandClient *>clients() const;
    Q_INVOKABLE void destroyClientForSurface(Aurora::Compositor::WaylandSurface *surface);
    Q_INVOKABLE void destroyClient(Aurora::Compositor::WaylandClient *client);

    QList<WaylandSurface *> surfaces() const;
    QList<WaylandSurface *> surfacesForClient(WaylandClient* client) const;

    Q_INVOKABLE Aurora::Compositor::WaylandOutput *outputFor(QWindow *window) const;

    WaylandOutput *defaultOutput() const;
    void setDefaultOutput(WaylandOutput *output);
    QList<WaylandOutput *> outputs() const;

    uint currentTimeMsecs() const;

    void setRetainedSelectionEnabled(bool enabled);
    bool retainedSelectionEnabled() const;
    void overrideSelection(const QMimeData *data);

    WaylandSeat *defaultSeat() const;

    WaylandSeat *seatFor(QInputEvent *inputEvent);

    bool useHardwareIntegrationExtension() const;
    void setUseHardwareIntegrationExtension(bool use);

    QVector<ShmFormat> additionalShmFormats() const;
    void setAdditionalShmFormats(const QVector<ShmFormat> &additionalShmFormats);

    virtual void grabSurface(WaylandSurfaceGrabber *grabber, const WaylandBufferRef &buffer);

public Q_SLOTS:
    void processWaylandEvents();

private Q_SLOTS:
    void applicationStateChanged(Qt::ApplicationState state);

Q_SIGNALS:
    void createdChanged();
    void socketNameChanged(const QByteArray &socketName);
    void retainedSelectionChanged(bool retainedSelection);

    void surfaceRequested(Aurora::Compositor::WaylandClient *client, uint id, int version);
    void surfaceCreated(Aurora::Compositor::WaylandSurface *surface);
    void surfaceAboutToBeDestroyed(Aurora::Compositor::WaylandSurface *surface);
    void subsurfaceChanged(Aurora::Compositor::WaylandSurface *child, Aurora::Compositor::WaylandSurface *parent);

    void defaultOutputChanged();
    void defaultSeatChanged(Aurora::Compositor::WaylandSeat *newDevice, Aurora::Compositor::WaylandSeat *oldDevice);

    void useHardwareIntegrationExtensionChanged();

    void outputAdded(Aurora::Compositor::WaylandOutput *output);
    void outputRemoved(Aurora::Compositor::WaylandOutput *output);

    void additionalShmFormatsChanged();

protected:
    virtual void retainedSelectionReceived(QMimeData *mimeData);
    virtual WaylandSeat *createSeat();
    virtual WaylandPointer *createPointerDevice(WaylandSeat *seat);
    virtual WaylandKeyboard *createKeyboardDevice(WaylandSeat *seat);
    virtual WaylandTouch *createTouchDevice(WaylandSeat *seat);

private:
    QScopedPointer<WaylandCompositorPrivate> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDCOMPOSITOR_H
