// SPDX-FileCopyrightText: 2021-2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QDir>
#include <QFile>
#include <QSet>
#include <QSettings>
#include <QSharedData>
#include <QStack>
#include <QStandardPaths>

#include "xcursor.h"
#include "xcursortheme_p.h"

namespace Aurora {

namespace Core {

class XcursorSpritePrivate : public QSharedData
{
public:
    QImage data;
    QPoint hotSpot;
    std::chrono::milliseconds delay;
};

class XcursorThemePrivate : public QSharedData
{
public:
    void load(const QString &themeName, int size, qreal devicePixelRatio);
    void loadCursors(const QString &packagePath, int size, qreal devicePixelRatio);

    QHash<QByteArray, QVector<XcursorSprite>> registry;
};

/*
 * XcursorSprite
 */

XcursorSprite::XcursorSprite()
    : d(new XcursorSpritePrivate)
{
}

XcursorSprite::XcursorSprite(const XcursorSprite &other)
    : d(other.d)
{
}

XcursorSprite::~XcursorSprite()
{
}

XcursorSprite &XcursorSprite::operator=(const XcursorSprite &other)
{
    d = other.d;
    return *this;
}

XcursorSprite::XcursorSprite(const QImage &data, const QPoint &hotSpot,
                             const std::chrono::milliseconds &delay)
    : d(new XcursorSpritePrivate)
{
    d->data = data;
    d->hotSpot = hotSpot;
    d->delay = delay;
}

QImage XcursorSprite::data() const
{
    return d->data;
}

QPoint XcursorSprite::hotSpot() const
{
    return d->hotSpot;
}

std::chrono::milliseconds XcursorSprite::delay() const
{
    return d->delay;
}

static QVector<XcursorSprite> loadCursor(const QString &filePath, int desiredSize,
                                         qreal devicePixelRatio)
{
    XcursorImages *images =
            XcursorFileLoadImages(QFile::encodeName(filePath).constData(), desiredSize * devicePixelRatio);
    if (!images) {
        return {};
    }

    QVector<XcursorSprite> sprites;
    for (int i = 0; i < images->nimage; ++i) {
        const XcursorImage *nativeCursorImage = images->images[i];
        const qreal scale = std::max(qreal(1), qreal(nativeCursorImage->size) / desiredSize);
        const QPoint hotspot(nativeCursorImage->xhot, nativeCursorImage->yhot);
        const std::chrono::milliseconds delay(nativeCursorImage->delay);

        QImage data(nativeCursorImage->width, nativeCursorImage->height,
                    QImage::Format_ARGB32_Premultiplied);
        data.setDevicePixelRatio(scale);
        memcpy(data.bits(), nativeCursorImage->pixels, data.sizeInBytes());

        sprites.append(XcursorSprite(data, hotspot / scale, delay));
    }

    XcursorImagesDestroy(images);
    return sprites;
}

void XcursorThemePrivate::loadCursors(const QString &packagePath, int size, qreal devicePixelRatio)
{
    const QDir dir(packagePath);
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    std::partition(entries.begin(), entries.end(),
                   [](const QFileInfo &fileInfo) { return !fileInfo.isSymLink(); });

    for (const QFileInfo &entry : std::as_const(entries)) {
        const QByteArray shape = QFile::encodeName(entry.fileName());
        if (registry.contains(shape)) {
            continue;
        }
        if (entry.isSymLink()) {
            const QFileInfo symLinkInfo(entry.symLinkTarget());
            if (symLinkInfo.absolutePath() == entry.absolutePath()) {
                const auto sprites = registry.value(QFile::encodeName(symLinkInfo.fileName()));
                if (!sprites.isEmpty()) {
                    registry.insert(shape, sprites);
                    continue;
                }
            }
        }
        const QVector<XcursorSprite> sprites =
                loadCursor(entry.absoluteFilePath(), size, devicePixelRatio);
        if (!sprites.isEmpty())
            registry.insert(shape, sprites);
    }
}

static QStringList searchPaths()
{
    static QStringList paths;

    if (paths.isEmpty()) {
        if (const QString env = qEnvironmentVariable("XCURSOR_PATH"); !env.isEmpty()) {
            paths.append(env.split(QLatin1Char(':'), Qt::SkipEmptyParts));
        } else {
            const QString home = QDir::homePath();
            if (!home.isEmpty()) {
                paths.append(home + QLatin1String("/.icons"));
            }
            const QStringList dataDirs =
                    QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
            for (const QString &dataDir : dataDirs) {
                paths.append(dataDir + QLatin1String("/icons"));
            }
        }
    }
    return paths;
}

void XcursorThemePrivate::load(const QString &themeName, int size, qreal devicePixelRatio)
{
    const QStringList paths = searchPaths();

    QStack<QString> stack;
    QSet<QString> loaded;

    stack.push(themeName);

    while (!stack.isEmpty()) {
        const QString themeName = stack.pop();
        if (loaded.contains(themeName)) {
            continue;
        }

        QStringList inherits;

        for (const QString &path : paths) {
            const QDir dir(path + QLatin1Char('/') + themeName);
            if (!dir.exists()) {
                continue;
            }
            loadCursors(dir.filePath(QStringLiteral("cursors")), size, devicePixelRatio);
            if (inherits.isEmpty()) {
                auto settings = QSettings(dir.filePath(QStringLiteral("index.theme")),
                                          QSettings::IniFormat);
                settings.beginGroup(QStringLiteral("Icon Theme"));
                inherits
                        << settings.value(QStringLiteral("Inherits"), QStringList()).toStringList();
            }
        }

        loaded.insert(themeName);
        for (auto it = inherits.crbegin(); it != inherits.crend(); ++it) {
            stack.push(*it);
        }
    }
}

/*
 * XcursorTheme
 */

XcursorTheme::XcursorTheme()
    : d(new XcursorThemePrivate)
{
}

XcursorTheme::XcursorTheme(const QString &themeName, int size, qreal devicePixelRatio)
    : d(new XcursorThemePrivate)
{
    d->load(themeName, size, devicePixelRatio);
}

XcursorTheme::XcursorTheme(const XcursorTheme &other)
    : d(other.d)
{
}

XcursorTheme::~XcursorTheme()
{
}

XcursorTheme &XcursorTheme::operator=(const XcursorTheme &other)
{
    d = other.d;
    return *this;
}

bool XcursorTheme::operator==(const XcursorTheme &other)
{
    return d == other.d;
}

bool XcursorTheme::operator!=(const XcursorTheme &other)
{
    return !(*this == other);
}

bool XcursorTheme::isEmpty() const
{
    return d->registry.isEmpty();
}

QVector<XcursorSprite> XcursorTheme::shape(const QByteArray &name) const
{
    return d->registry.value(name);
}

} // namespace Core

} // namespace Aurora
