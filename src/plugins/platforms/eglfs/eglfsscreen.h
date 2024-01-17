// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPointer>

#include <qpa/qplatformscreen.h>

#include <LiriAuroraPlatform/Output>

class QOpenGLContext;

class EglFSCursor;
class EglFSWindow;

class EglFSScreen
    : public QObject
    , public QPlatformScreen
{
    Q_OBJECT
public:
    EglFSScreen(Aurora::Platform::Output *output);
    ~EglFSScreen();

    Aurora::Platform::Output *auroraOutput() const;

    QString name() const override;

    QString manufacturer() const override;
    QString model() const override;
    QString serialNumber() const override;

    QRect geometry() const override;

    qreal refreshRate() const override;

    int depth() const override;
    QImage::Format format() const override;

    QSizeF physicalSize() const override;
    QDpi logicalDpi() const override;
    QDpi logicalBaseDpi() const override;

    Qt::ScreenOrientation nativeOrientation() const override;
    Qt::ScreenOrientation orientation() const override;

    QPlatformCursor *cursor() const override;

    QPlatformScreen::SubpixelAntialiasingType subpixelAntialiasingTypeHint() const override;

    QPixmap grabWindow(WId wid, int x, int y, int width, int height) const override;

protected:
    QPointer<Aurora::Platform::Output> m_output;
    EglFSCursor *m_cursor = nullptr;
};
