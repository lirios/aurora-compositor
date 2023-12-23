// SPDX-FileCopyrightText: 2019-2022 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <LiriAuroraCompositor/WaylandCompositor>
#include <LiriAuroraCompositor/WaylandCompositorExtension>
#include <LiriAuroraCompositor/WaylandOutput>
#include <LiriAuroraCompositor/WaylandResource>
#include <LiriAuroraCompositor/aurorawaylandquickchildren.h>

namespace Aurora {

namespace Compositor {

class WaylandClient;
class WaylandWlrOutputConfigurationV1;
class WaylandWlrOutputConfigurationV1Private;
class WaylandWlrOutputConfigurationHeadV1;
class WaylandWlrOutputConfigurationHeadV1Private;
class WaylandWlrOutputManagerV1Private;
class WaylandWlrOutputHeadV1;
class WaylandWlrOutputHeadV1Private;
class WaylandWlrOutputModeV1;
class WaylandWlrOutputModeV1Private;

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrOutputManagerV1
        : public WaylandCompositorExtensionTemplate<WaylandWlrOutputManagerV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrOutputManagerV1)
    Q_PROPERTY(Aurora::Compositor::WaylandCompositor *compositor READ compositor WRITE setCompositor NOTIFY compositorChanged)
public:
    explicit WaylandWlrOutputManagerV1();
    WaylandWlrOutputManagerV1(WaylandCompositor *compositor);
    ~WaylandWlrOutputManagerV1();

    void initialize() override;

    WaylandCompositor *compositor() const;
    void setCompositor(WaylandCompositor *compositor);

    QVector<WaylandWlrOutputHeadV1 *> heads() const;

    Q_INVOKABLE void done(quint32 serial);
    Q_INVOKABLE void finished();

    static const wl_interface *interface();
    static QByteArray interfaceName();

Q_SIGNALS:
    void compositorChanged();
    void headAdded(Aurora::Compositor::WaylandWlrOutputHeadV1 *head);
    void configurationRequested(const Aurora::Compositor::WaylandResource &resource);
    void configurationCreated(Aurora::Compositor::WaylandWlrOutputConfigurationV1 *configuration);
    void clientStopped(Aurora::Compositor::WaylandClient *client);

private:
    QScopedPointer<WaylandWlrOutputManagerV1Private> const d_ptr;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrOutputHeadV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrOutputHeadV1)
    Q_PROPERTY(Aurora::Compositor::WaylandWlrOutputManagerV1 *manager READ manager WRITE setManager NOTIFY managerChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QSize physicalSize READ physicalSize WRITE setPhysicalSize NOTIFY physicalSizeChanged)
    Q_PROPERTY(QPoint position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandWlrOutputModeV1 *currentMode READ currentMode WRITE setCurrentMode NOTIFY currentModeChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandWlrOutputModeV1 *preferredMode READ preferredMode WRITE setPreferredMode NOTIFY preferredModeChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandOutput::Transform transform READ transform WRITE setTransform NOTIFY transformChanged)
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)
public:
    explicit WaylandWlrOutputHeadV1(QObject *parent = nullptr);
    ~WaylandWlrOutputHeadV1();

    bool isInitialized() const;
    void initialize();

    WaylandWlrOutputManagerV1 *manager() const;
    void setManager(WaylandWlrOutputManagerV1 *manager);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    QString name() const;
    void setName(const QString &name);

    QString description() const;
    void setDescription(const QString &description);

    QSize physicalSize() const;
    void setPhysicalSize(const QSize &physicalSize);

    QPoint position() const;
    void setPosition(const QPoint &position);

    QVector<WaylandWlrOutputModeV1 *> modes() const;
    Q_INVOKABLE void addMode(Aurora::Compositor::WaylandWlrOutputModeV1 *mode);

    WaylandWlrOutputModeV1 *currentMode() const;
    void setCurrentMode(WaylandWlrOutputModeV1 *mode);

    WaylandWlrOutputModeV1 *preferredMode() const;
    void setPreferredMode(WaylandWlrOutputModeV1 *mode);

    WaylandOutput::Transform transform() const;
    void setTransform(WaylandOutput::Transform transform);

    qreal scale() const;
    void setScale(qreal scale);

Q_SIGNALS:
    void managerChanged();
    void enabledChanged();
    void nameChanged();
    void descriptionChanged();
    void physicalSizeChanged();
    void positionChanged();
    void modeAdded(Aurora::Compositor::WaylandWlrOutputModeV1 *mode);
    void modesChanged();
    void currentModeChanged();
    void preferredModeChanged();
    void transformChanged();
    void scaleChanged();

private:
    QScopedPointer<WaylandWlrOutputHeadV1Private> const d_ptr;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrOutputModeV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrOutputModeV1)
    Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(qint32 refresh READ refresh WRITE setRefresh NOTIFY refreshChanged)
    AURORA_COMPOSITOR_DECLARE_QUICK_CHILDREN(WaylandWlrOutputModeV1)
    public:
        explicit WaylandWlrOutputModeV1(QObject *parent = nullptr);
    ~WaylandWlrOutputModeV1();

    bool isInitialized() const;

    QSize size() const;
    void setSize(const QSize &size);

    qint32 refresh() const;
    void setRefresh(qint32 refresh);

Q_SIGNALS:
    void sizeChanged();
    void refreshChanged();

private:
    QScopedPointer<WaylandWlrOutputModeV1Private> const d_ptr;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrOutputConfigurationHeadV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrOutputConfigurationHeadV1)
    Q_PROPERTY(Aurora::Compositor::WaylandWlrOutputHeadV1 *head READ head CONSTANT)
    Q_PROPERTY(Aurora::Compositor::WaylandWlrOutputModeV1 *mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(QSize customModeSize READ customModeSize NOTIFY customModeChanged)
    Q_PROPERTY(qint32 customModeRefresh READ customModeRefresh NOTIFY customModeChanged)
    Q_PROPERTY(QPoint position READ position NOTIFY positionChanged)
    Q_PROPERTY(Aurora::Compositor::WaylandOutput::Transform transform READ transform NOTIFY transformChanged)
    Q_PROPERTY(qreal scale READ scale NOTIFY scaleChanged)
public:
    ~WaylandWlrOutputConfigurationHeadV1();

    WaylandWlrOutputHeadV1 *head() const;
    WaylandWlrOutputModeV1 *mode() const;
    QSize customModeSize() const;
    qint32 customModeRefresh() const;
    QPoint position() const;
    WaylandOutput::Transform transform() const;
    qreal scale() const;

Q_SIGNALS:
    void modeChanged(Aurora::Compositor::WaylandWlrOutputModeV1 *mode);
    void customModeChanged(const QSize &size, qint32 refreshRate);
    void positionChanged(const QPoint &position);
    void transformChanged(Aurora::Compositor::WaylandOutput::Transform transform);
    void scaleChanged(qreal scale);

private:
    QScopedPointer<WaylandWlrOutputConfigurationHeadV1Private> const d_ptr;

    explicit WaylandWlrOutputConfigurationHeadV1(WaylandWlrOutputHeadV1 *head,
                                                 QObject *parent = nullptr);

    friend class WaylandWlrOutputConfigurationV1Private;
};

class LIRIAURORACOMPOSITOR_EXPORT WaylandWlrOutputConfigurationV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WaylandWlrOutputConfigurationV1)
public:
    explicit WaylandWlrOutputConfigurationV1(QObject *parent = nullptr);
    WaylandWlrOutputConfigurationV1(WaylandWlrOutputManagerV1 *manager,
                                    const WaylandResource &resource);
    ~WaylandWlrOutputConfigurationV1();

    QVector<WaylandWlrOutputConfigurationHeadV1 *> enabledHeads() const;
    QVector<WaylandWlrOutputHeadV1 *> disabledHeads() const;

    Q_INVOKABLE void initialize(Aurora::Compositor::WaylandWlrOutputManagerV1 *manager,
                                const Aurora::Compositor::WaylandResource &resource);

    Q_INVOKABLE void sendSucceeded();
    Q_INVOKABLE void sendFailed();
    Q_INVOKABLE void sendCancelled();

    static WaylandWlrOutputConfigurationV1 *fromResource(struct ::wl_resource *resource);

Q_SIGNALS:
    void enabledHeadsChanged();
    void disabledHeadsChanged();
    void headEnabled(Aurora::Compositor::WaylandWlrOutputConfigurationHeadV1 *headChanges);
    void headDisabled(Aurora::Compositor::WaylandWlrOutputHeadV1 *head);
    void readyToApply();
    void readyToTest();

private:
    QScopedPointer<WaylandWlrOutputConfigurationV1Private> const d_ptr;
};

} // namespace Compositor

} // namespace Aurora

