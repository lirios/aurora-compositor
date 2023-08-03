// SPDX-FileCopyrightText: 2023 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QByteArrayList>
#include <QHash>
#include <QTimer>

#include "shapecursorsource.h"
#include "xcursortheme_p.h"

static QByteArray cursorShapeToName(Qt::CursorShape shape)
{
    switch (shape) {
    case Qt::ArrowCursor:
        return QByteArrayLiteral("left_ptr");
    case Qt::UpArrowCursor:
        return QByteArrayLiteral("up_arrow");
    case Qt::CrossCursor:
        return QByteArrayLiteral("cross");
    case Qt::WaitCursor:
        return QByteArrayLiteral("wait");
    case Qt::IBeamCursor:
        return QByteArrayLiteral("ibeam");
    case Qt::SizeVerCursor:
        return QByteArrayLiteral("size_ver");
    case Qt::SizeHorCursor:
        return QByteArrayLiteral("size_hor");
    case Qt::SizeBDiagCursor:
        return QByteArrayLiteral("size_bdiag");
    case Qt::SizeFDiagCursor:
        return QByteArrayLiteral("size_fdiag");
    case Qt::SizeAllCursor:
        return QByteArrayLiteral("size_all");
    case Qt::SplitVCursor:
        return QByteArrayLiteral("split_v");
    case Qt::SplitHCursor:
        return QByteArrayLiteral("split_h");
    case Qt::PointingHandCursor:
        return QByteArrayLiteral("pointing_hand");
    case Qt::ForbiddenCursor:
        return QByteArrayLiteral("forbidden");
    case Qt::OpenHandCursor:
        return QByteArrayLiteral("openhand");
    case Qt::ClosedHandCursor:
        return QByteArrayLiteral("closedhand");
    case Qt::WhatsThisCursor:
        return QByteArrayLiteral("whats_this");
    case Qt::BusyCursor:
        return QByteArrayLiteral("left_ptr_watch");
    case Qt::DragMoveCursor:
        return QByteArrayLiteral("dnd-move");
    case Qt::DragCopyCursor:
        return QByteArrayLiteral("dnd-copy");
    case Qt::DragLinkCursor:
        return QByteArrayLiteral("dnd-link");
#if 0
    case KWin::ExtendedCursor::SizeNorthEast:
        return QByteArrayLiteral("ne-resize");
    case KWin::ExtendedCursor::SizeNorth:
        return QByteArrayLiteral("n-resize");
    case KWin::ExtendedCursor::SizeNorthWest:
        return QByteArrayLiteral("nw-resize");
    case KWin::ExtendedCursor::SizeEast:
        return QByteArrayLiteral("e-resize");
    case KWin::ExtendedCursor::SizeWest:
        return QByteArrayLiteral("w-resize");
    case KWin::ExtendedCursor::SizeSouthEast:
        return QByteArrayLiteral("se-resize");
    case KWin::ExtendedCursor::SizeSouth:
        return QByteArrayLiteral("s-resize");
    case KWin::ExtendedCursor::SizeSouthWest:
        return QByteArrayLiteral("sw-resize");
#endif
    default:
        return QByteArray();
    }
}

static QByteArrayList cursorAlternativeNames(const QByteArray &name)
{
    static const QHash<QByteArray, QByteArrayList> alternatives = {
        {
                QByteArrayLiteral("left_ptr"),
                {
                        QByteArrayLiteral("arrow"),
                        QByteArrayLiteral("dnd-none"),
                        QByteArrayLiteral("op_left_arrow"),
                },
        },
        {
                QByteArrayLiteral("cross"),
                {
                        QByteArrayLiteral("crosshair"),
                        QByteArrayLiteral("diamond-cross"),
                        QByteArrayLiteral("cross-reverse"),
                },
        },
        {
                QByteArrayLiteral("up_arrow"),
                {
                        QByteArrayLiteral("center_ptr"),
                        QByteArrayLiteral("sb_up_arrow"),
                        QByteArrayLiteral("centre_ptr"),
                },
        },
        {
                QByteArrayLiteral("wait"),
                {
                        QByteArrayLiteral("watch"),
                        QByteArrayLiteral("progress"),
                },
        },
        {
                QByteArrayLiteral("ibeam"),
                {
                        QByteArrayLiteral("xterm"),
                        QByteArrayLiteral("text"),
                },
        },
        {
                QByteArrayLiteral("size_all"),
                {
                        QByteArrayLiteral("fleur"),
                },
        },
        {
                QByteArrayLiteral("pointing_hand"),
                {
                        QByteArrayLiteral("hand2"),
                        QByteArrayLiteral("hand"),
                        QByteArrayLiteral("hand1"),
                        QByteArrayLiteral("pointer"),
                        QByteArrayLiteral("e29285e634086352946a0e7090d73106"),
                        QByteArrayLiteral("9d800788f1b08800ae810202380a0822"),
                },
        },
        {
                QByteArrayLiteral("size_ver"),
                {
                        QByteArrayLiteral("00008160000006810000408080010102"),
                        QByteArrayLiteral("sb_v_double_arrow"),
                        QByteArrayLiteral("v_double_arrow"),
                        QByteArrayLiteral("n-resize"),
                        QByteArrayLiteral("s-resize"),
                        QByteArrayLiteral("col-resize"),
                        QByteArrayLiteral("top_side"),
                        QByteArrayLiteral("bottom_side"),
                        QByteArrayLiteral("base_arrow_up"),
                        QByteArrayLiteral("base_arrow_down"),
                        QByteArrayLiteral("based_arrow_down"),
                        QByteArrayLiteral("based_arrow_up"),
                },
        },
        {
                QByteArrayLiteral("size_hor"),
                {
                        QByteArrayLiteral("028006030e0e7ebffc7f7070c0600140"),
                        QByteArrayLiteral("sb_h_double_arrow"),
                        QByteArrayLiteral("h_double_arrow"),
                        QByteArrayLiteral("e-resize"),
                        QByteArrayLiteral("w-resize"),
                        QByteArrayLiteral("row-resize"),
                        QByteArrayLiteral("right_side"),
                        QByteArrayLiteral("left_side"),
                },
        },
        {
                QByteArrayLiteral("size_bdiag"),
                {
                        QByteArrayLiteral("fcf1c3c7cd4491d801f1e1c78f100000"),
                        QByteArrayLiteral("fd_double_arrow"),
                        QByteArrayLiteral("bottom_left_corner"),
                        QByteArrayLiteral("top_right_corner"),
                },
        },
        {
                QByteArrayLiteral("size_fdiag"),
                {
                        QByteArrayLiteral("c7088f0f3e6c8088236ef8e1e3e70000"),
                        QByteArrayLiteral("bd_double_arrow"),
                        QByteArrayLiteral("bottom_right_corner"),
                        QByteArrayLiteral("top_left_corner"),
                },
        },
        {
                QByteArrayLiteral("whats_this"),
                {
                        QByteArrayLiteral("d9ce0ab605698f320427677b458ad60b"),
                        QByteArrayLiteral("left_ptr_help"),
                        QByteArrayLiteral("help"),
                        QByteArrayLiteral("question_arrow"),
                        QByteArrayLiteral("dnd-ask"),
                        QByteArrayLiteral("5c6cd98b3f3ebcb1f9c7f1c204630408"),
                },
        },
        {
                QByteArrayLiteral("split_h"),
                {
                        QByteArrayLiteral("14fef782d02440884392942c11205230"),
                        QByteArrayLiteral("size_hor"),
                },
        },
        {
                QByteArrayLiteral("split_v"),
                {
                        QByteArrayLiteral("2870a09082c103050810ffdffffe0204"),
                        QByteArrayLiteral("size_ver"),
                },
        },
        {
                QByteArrayLiteral("forbidden"),
                {
                        QByteArrayLiteral("03b6e0fcb3499374a867c041f52298f0"),
                        QByteArrayLiteral("circle"),
                        QByteArrayLiteral("dnd-no-drop"),
                        QByteArrayLiteral("not-allowed"),
                },
        },
        {
                QByteArrayLiteral("left_ptr_watch"),
                {
                        QByteArrayLiteral("3ecb610c1bf2410f44200f48c40d3599"),
                        QByteArrayLiteral("00000000000000020006000e7e9ffc3f"),
                        QByteArrayLiteral("08e8e1c95fe2fc01f976f1e063a24ccd"),
                },
        },
        {
                QByteArrayLiteral("openhand"),
                {
                        QByteArrayLiteral("9141b49c8149039304290b508d208c40"),
                        QByteArrayLiteral("all_scroll"),
                        QByteArrayLiteral("all-scroll"),
                },
        },
        {
                QByteArrayLiteral("closedhand"),
                {
                        QByteArrayLiteral("05e88622050804100c20044008402080"),
                        QByteArrayLiteral("4498f0e0c1937ffe01fd06f973665830"),
                        QByteArrayLiteral("9081237383d90e509aa00f00170e968f"),
                        QByteArrayLiteral("fcf21c00b30f7e3f83fe0dfd12e71cff"),
                },
        },
        {
                QByteArrayLiteral("dnd-link"),
                {
                        QByteArrayLiteral("link"),
                        QByteArrayLiteral("alias"),
                        QByteArrayLiteral("3085a0e285430894940527032f8b26df"),
                        QByteArrayLiteral("640fb0e74195791501fd1ed57b41487f"),
                        QByteArrayLiteral("a2a266d0498c3104214a47bd64ab0fc8"),
                },
        },
        {
                QByteArrayLiteral("dnd-copy"),
                {
                        QByteArrayLiteral("copy"),
                        QByteArrayLiteral("1081e37283d90000800003c07f3ef6bf"),
                        QByteArrayLiteral("6407b0e94181790501fd1e167b474872"),
                        QByteArrayLiteral("b66166c04f8c3109214a4fbd64a50fc8"),
                },
        },
        {
                QByteArrayLiteral("dnd-move"),
                {
                        QByteArrayLiteral("move"),
                },
        },
        {
                QByteArrayLiteral("sw-resize"),
                {
                        QByteArrayLiteral("size_bdiag"),
                        QByteArrayLiteral("fcf1c3c7cd4491d801f1e1c78f100000"),
                        QByteArrayLiteral("fd_double_arrow"),
                        QByteArrayLiteral("bottom_left_corner"),
                },
        },
        {
                QByteArrayLiteral("se-resize"),
                {
                        QByteArrayLiteral("size_fdiag"),
                        QByteArrayLiteral("c7088f0f3e6c8088236ef8e1e3e70000"),
                        QByteArrayLiteral("bd_double_arrow"),
                        QByteArrayLiteral("bottom_right_corner"),
                },
        },
        {
                QByteArrayLiteral("ne-resize"),
                {
                        QByteArrayLiteral("size_bdiag"),
                        QByteArrayLiteral("fcf1c3c7cd4491d801f1e1c78f100000"),
                        QByteArrayLiteral("fd_double_arrow"),
                        QByteArrayLiteral("top_right_corner"),
                },
        },
        {
                QByteArrayLiteral("nw-resize"),
                {
                        QByteArrayLiteral("size_fdiag"),
                        QByteArrayLiteral("c7088f0f3e6c8088236ef8e1e3e70000"),
                        QByteArrayLiteral("bd_double_arrow"),
                        QByteArrayLiteral("top_left_corner"),
                },
        },
        {
                QByteArrayLiteral("n-resize"),
                {
                        QByteArrayLiteral("size_ver"),
                        QByteArrayLiteral("00008160000006810000408080010102"),
                        QByteArrayLiteral("sb_v_double_arrow"),
                        QByteArrayLiteral("v_double_arrow"),
                        QByteArrayLiteral("col-resize"),
                        QByteArrayLiteral("top_side"),
                },
        },
        {
                QByteArrayLiteral("e-resize"),
                {
                        QByteArrayLiteral("size_hor"),
                        QByteArrayLiteral("028006030e0e7ebffc7f7070c0600140"),
                        QByteArrayLiteral("sb_h_double_arrow"),
                        QByteArrayLiteral("h_double_arrow"),
                        QByteArrayLiteral("row-resize"),
                        QByteArrayLiteral("left_side"),
                },
        },
        {
                QByteArrayLiteral("s-resize"),
                {
                        QByteArrayLiteral("size_ver"),
                        QByteArrayLiteral("00008160000006810000408080010102"),
                        QByteArrayLiteral("sb_v_double_arrow"),
                        QByteArrayLiteral("v_double_arrow"),
                        QByteArrayLiteral("col-resize"),
                        QByteArrayLiteral("bottom_side"),
                },
        },
        {
                QByteArrayLiteral("w-resize"),
                {
                        QByteArrayLiteral("size_hor"),
                        QByteArrayLiteral("028006030e0e7ebffc7f7070c0600140"),
                        QByteArrayLiteral("sb_h_double_arrow"),
                        QByteArrayLiteral("h_double_arrow"),
                        QByteArrayLiteral("right_side"),
                },
        },
    };
    auto it = alternatives.find(name);
    if (it != alternatives.end()) {
        return it.value();
    }
    return QByteArrayList();
}

namespace Aurora {

namespace Core {

/*
 * ShapeCursorSourcePrivate
 */

class ShapeCursorSourcePrivate
{
    Q_DECLARE_PUBLIC(ShapeCursorSource)
public:
    explicit ShapeCursorSourcePrivate(ShapeCursorSource *self);

    void refresh();
    void selectSprite(int index);
    void selectNextSprite();

    QSizeF size;
    QPointF hotSpot;

    XcursorTheme xcursorTheme;
    QTimer delayTimer;
    QImage image;
    QByteArray shape;
    int currentSprite = -1;
    QVector<XcursorSprite> sprites;

protected:
    ShapeCursorSource *q_ptr = nullptr;
};

ShapeCursorSourcePrivate::ShapeCursorSourcePrivate(ShapeCursorSource *self)
    : q_ptr(self)
{
    delayTimer.setSingleShot(true);
    QObject::connect(&delayTimer, SIGNAL(timeout()), self, SLOT(selectNextSprite()));
}

void ShapeCursorSourcePrivate::refresh()
{
    currentSprite = -1;
    delayTimer.stop();

    sprites = xcursorTheme.shape(shape);
    if (sprites.isEmpty()) {
        const auto alternativeNames = cursorAlternativeNames(shape);
        for (const QByteArray &alternativeName : alternativeNames) {
            sprites = xcursorTheme.shape(alternativeName);
            if (!sprites.isEmpty())
                break;
        }
    }

    if (!sprites.isEmpty())
        selectSprite(0);
}

void ShapeCursorSourcePrivate::selectSprite(int index)
{
    Q_Q(ShapeCursorSource);

    if (currentSprite == index)
        return;

    const XcursorSprite &sprite = sprites[index];
    currentSprite = index;
    image = sprite.data();
    size = QSizeF(image.size()) / image.devicePixelRatio();
    hotSpot = sprite.hotSpot();

    if (sprite.delay().count() && sprites.size() > 1)
        delayTimer.start(sprite.delay());

    emit q->changed();
}

void ShapeCursorSourcePrivate::selectNextSprite()
{
    selectSprite((currentSprite + 1) % sprites.size());
}

/*
 * ShapeCursorSource
 */

ShapeCursorSource::ShapeCursorSource(QObject *parent)
    : CursorSource(parent)
    , d_ptr(new ShapeCursorSourcePrivate(this))
{
}

ShapeCursorSource::~ShapeCursorSource()
{
}

QSizeF ShapeCursorSource::size() const
{
    Q_D(const ShapeCursorSource);
    return d->size;
}

QPointF ShapeCursorSource::hotSpot() const
{
    Q_D(const ShapeCursorSource);
    return d->hotSpot;
}

QImage ShapeCursorSource::image() const
{
    Q_D(const ShapeCursorSource);
    return d->image;
}

QByteArray ShapeCursorSource::shape() const
{
    Q_D(const ShapeCursorSource);
    return d->shape;
}

void ShapeCursorSource::setShape(const QByteArray &shape)
{
    Q_D(ShapeCursorSource);

    if (d->shape != shape) {
        d->shape = shape;
        d->refresh();
    }
}

void ShapeCursorSource::setShape(Qt::CursorShape shape)
{
    setShape(cursorShapeToName(shape));
}

void ShapeCursorSource::loadTheme(const QString &themeName, int size, qreal devicePixelRatio)
{
    Q_D(ShapeCursorSource);
    d->xcursorTheme = XcursorTheme(themeName, size, devicePixelRatio);
}

} // namespace Core

} // namespace Aurora

#include "moc_shapecursorsource.cpp"