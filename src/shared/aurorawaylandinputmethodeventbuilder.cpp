/****************************************************************************
**
** Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "aurorawaylandinputmethodeventbuilder_p.h"

#include <QBrush>
#include <QGuiApplication>
#include <QInputMethod>
#include <QPalette>
#include <QTextCharFormat>

#ifdef LIRI_BUILD_AURORACOMPOSITOR_LIB
#include <LiriAuroraCompositor/private/aurora-server-text-input-unstable-v2.h>
#include <LiriAuroraCompositor/private/aurora-server-text-input-unstable-v4-wip.h>
#else
#include <QtWaylandClient/private/aurora-client-text-input-unstable-v2.h>
#include <QtWaylandClient/private/aurora-client-text-input-unstable-v4-wip.h>
#endif

namespace Aurora {

namespace Compositor {

WaylandInputMethodEventBuilder::~WaylandInputMethodEventBuilder()
{
}

void WaylandInputMethodEventBuilder::reset()
{
    m_anchor = 0;
    m_cursor = 0;
    m_deleteBefore = 0;
    m_deleteAfter = 0;
    m_preeditCursor = 0;
    m_preeditStyles.clear();
}

void WaylandInputMethodEventBuilder::setCursorPosition(int32_t index, int32_t anchor)
{
    m_cursor = index;
    m_anchor = anchor;
}

void WaylandInputMethodEventBuilder::setDeleteSurroundingText(uint32_t beforeLength, uint32_t afterLength)
{
    m_deleteBefore = beforeLength;
    m_deleteAfter = afterLength;
}

void WaylandInputMethodEventBuilder::addPreeditStyling(uint32_t index, uint32_t length, uint32_t style)
{
    QTextCharFormat format;

    switch (style) {
    case ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_NONE:
        break;
    case ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_DEFAULT:
    case ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_UNDERLINE:
        format.setFontUnderline(true);
        format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        m_preeditStyles.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, index, length, format));
        break;
    case ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_ACTIVE:
    case ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_INACTIVE:
        format.setFontWeight(QFont::Bold);
        format.setFontUnderline(true);
        format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        m_preeditStyles.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, index, length, format));
        break;
    case ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_HIGHLIGHT:
    case ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_SELECTION:
        {
            format.setFontUnderline(true);
            format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
            QPalette palette = qApp->palette();
            format.setBackground(QBrush(palette.color(QPalette::Active, QPalette::Highlight)));
            format.setForeground(QBrush(palette.color(QPalette::Active, QPalette::HighlightedText)));
            m_preeditStyles.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, index, length, format));
        }
        break;
    case ZWP_TEXT_INPUT_V2_PREEDIT_STYLE_INCORRECT:
        format.setFontUnderline(true);
        format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        format.setUnderlineColor(QColor(Qt::red));
        m_preeditStyles.append(QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, index, length, format));
        break;
    default:
        break;
    }
}

void WaylandInputMethodEventBuilder::setPreeditCursor(int32_t index)
{
    m_preeditCursor = index;
}

QInputMethodEvent *WaylandInputMethodEventBuilder::buildCommit(const QString &text)
{
    QList<QInputMethodEvent::Attribute> attributes;

    const QPair<int, int> replacement = replacementForDeleteSurrounding();

    if (m_cursor != 0 || m_anchor != 0) {
        QString surrounding = QInputMethod::queryFocusObject(Qt::ImSurroundingText, QVariant()).toString();
        const int cursor = QInputMethod::queryFocusObject(Qt::ImCursorPosition, QVariant()).toInt();
        const int anchor = QInputMethod::queryFocusObject(Qt::ImAnchorPosition, QVariant()).toInt();
        const int absoluteCursor = QInputMethod::queryFocusObject(Qt::ImAbsolutePosition, QVariant()).toInt();

        const int absoluteOffset = absoluteCursor - cursor;

        const int cursorAfterCommit = qMin(anchor, cursor) + replacement.first + text.length();
        surrounding.replace(qMin(anchor, cursor) + replacement.first,
                            qAbs(anchor - cursor) + replacement.second, text);

        attributes.push_back(QInputMethodEvent::Attribute(QInputMethodEvent::Selection,
                                                          indexFromWayland(surrounding, m_cursor, cursorAfterCommit) + absoluteOffset,
                                                          indexFromWayland(surrounding, m_anchor, cursorAfterCommit) + absoluteOffset,
                                                          QVariant()));
    }

    QInputMethodEvent *event = new QInputMethodEvent(QString(), attributes);
    event->setCommitString(text, replacement.first, replacement.second);

    return event;
}

QInputMethodEvent *WaylandInputMethodEventBuilder::buildPreedit(const QString &text)
{
    QList<QInputMethodEvent::Attribute> attributes;

    if (m_preeditCursor < 0) {
        attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, 0, 0, QVariant()));
    } else {
        attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, indexFromWayland(text, m_preeditCursor), 1, QVariant()));
    }

    for (const QInputMethodEvent::Attribute &attr : qAsConst(m_preeditStyles)) {
        int start = indexFromWayland(text, attr.start);
        int length = indexFromWayland(text, attr.start + attr.length) - start;
        attributes.append(QInputMethodEvent::Attribute(attr.type, start, length, attr.value));
    }

    QInputMethodEvent *event = new QInputMethodEvent(text, attributes);

    const QPair<int, int> replacement = replacementForDeleteSurrounding();
    event->setCommitString(QString(), replacement.first, replacement.second);

    return event;
}

QPair<int, int> WaylandInputMethodEventBuilder::replacementForDeleteSurrounding()
{
    if (m_deleteBefore == 0 && m_deleteAfter == 0)
        return QPair<int, int>(0, 0);

    const QString &surrounding = QInputMethod::queryFocusObject(Qt::ImSurroundingText, QVariant()).toString();
    const int cursor = QInputMethod::queryFocusObject(Qt::ImCursorPosition, QVariant()).toInt();
    const int anchor = QInputMethod::queryFocusObject(Qt::ImAnchorPosition, QVariant()).toInt();

    const int selectionStart = qMin(cursor, anchor);
    const int selectionEnd = qMax(cursor, anchor);

    const int deleteBefore = selectionStart - indexFromWayland(surrounding, -m_deleteBefore, selectionStart);
    const int deleteAfter = indexFromWayland(surrounding, m_deleteAfter, selectionEnd) - selectionEnd;

    return QPair<int, int>(-deleteBefore, deleteBefore + deleteAfter);
}

WaylandInputMethodContentType WaylandInputMethodContentType::convert(Qt::InputMethodHints hints)
{
    uint32_t hint = ZWP_TEXT_INPUT_V2_CONTENT_HINT_NONE;
    uint32_t purpose = ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_NORMAL;

    if (hints & Qt::ImhHiddenText)
        hint |= ZWP_TEXT_INPUT_V2_CONTENT_HINT_HIDDEN_TEXT;
    if (hints & Qt::ImhSensitiveData)
        hint |= ZWP_TEXT_INPUT_V2_CONTENT_HINT_SENSITIVE_DATA;
    if ((hints & Qt::ImhNoAutoUppercase) == 0)
        hint |= ZWP_TEXT_INPUT_V2_CONTENT_HINT_AUTO_CAPITALIZATION;
    if (hints & Qt::ImhPreferNumbers) {
        // Nothing yet
    }
    if (hints & Qt::ImhPreferUppercase)
        hint |= ZWP_TEXT_INPUT_V2_CONTENT_HINT_UPPERCASE;
    if (hints & Qt::ImhPreferLowercase)
        hint |= ZWP_TEXT_INPUT_V2_CONTENT_HINT_LOWERCASE;
    if ((hints & Qt::ImhNoPredictiveText) == 0) {
        hint |= (ZWP_TEXT_INPUT_V2_CONTENT_HINT_AUTO_COMPLETION
                | ZWP_TEXT_INPUT_V2_CONTENT_HINT_AUTO_CORRECTION);
    }

    if ((hints & Qt::ImhDate) && (hints & Qt::ImhTime) == 0)
        purpose = ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_DATE;
    else if ((hints & Qt::ImhDate) && (hints & Qt::ImhTime))
        purpose = ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_DATETIME;
    else if ((hints & Qt::ImhDate) == 0 && (hints & Qt::ImhTime))
        purpose = ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_TIME;

    if (hints & Qt::ImhPreferLatin)
        hint |= ZWP_TEXT_INPUT_V2_CONTENT_HINT_LATIN;
    if (hints & Qt::ImhMultiLine)
        hint |= ZWP_TEXT_INPUT_V2_CONTENT_HINT_MULTILINE;
    if (hints & Qt::ImhDigitsOnly)
        purpose = ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_DIGITS;
    if (hints & Qt::ImhFormattedNumbersOnly)
        purpose = ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_NUMBER;
    if (hints & Qt::ImhUppercaseOnly)
        hint |= ZWP_TEXT_INPUT_V2_CONTENT_HINT_UPPERCASE;
    if (hints & Qt::ImhLowercaseOnly)
        hint |= ZWP_TEXT_INPUT_V2_CONTENT_HINT_LOWERCASE;
    if (hints & Qt::ImhDialableCharactersOnly)
        purpose = ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_PHONE;
    if (hints & Qt::ImhEmailCharactersOnly)
       purpose = ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_EMAIL;
    if (hints & Qt::ImhUrlCharactersOnly)
       purpose = ZWP_TEXT_INPUT_V2_CONTENT_PURPOSE_URL;
    if (hints & Qt::ImhLatinOnly)
        hint |= ZWP_TEXT_INPUT_V2_CONTENT_HINT_LATIN;

    return WaylandInputMethodContentType{hint, purpose};
}

WaylandInputMethodContentType WaylandInputMethodContentType::convertV4(Qt::InputMethodHints hints)
{
    uint32_t hint = ZWP_TEXT_INPUT_V4_CONTENT_HINT_NONE;
    uint32_t purpose = ZWP_TEXT_INPUT_V4_CONTENT_PURPOSE_NORMAL;

    if (hints & Qt::ImhHiddenText)
        hint |= ZWP_TEXT_INPUT_V4_CONTENT_HINT_HIDDEN_TEXT;
    if (hints & Qt::ImhSensitiveData)
        hint |= ZWP_TEXT_INPUT_V4_CONTENT_HINT_SENSITIVE_DATA;
    if ((hints & Qt::ImhNoAutoUppercase) == 0)
        hint |= ZWP_TEXT_INPUT_V4_CONTENT_HINT_AUTO_CAPITALIZATION;
    if (hints & Qt::ImhPreferNumbers) {
        // Nothing yet
    }
    if (hints & Qt::ImhPreferUppercase)
        hint |= ZWP_TEXT_INPUT_V4_CONTENT_HINT_UPPERCASE;
    if (hints & Qt::ImhPreferLowercase)
        hint |= ZWP_TEXT_INPUT_V4_CONTENT_HINT_LOWERCASE;
    if ((hints & Qt::ImhNoPredictiveText) == 0) {
        hint |= (ZWP_TEXT_INPUT_V4_CONTENT_HINT_COMPLETION
                | ZWP_TEXT_INPUT_V4_CONTENT_HINT_SPELLCHECK);
    }

    if ((hints & Qt::ImhDate) && (hints & Qt::ImhTime) == 0)
        purpose = ZWP_TEXT_INPUT_V4_CONTENT_PURPOSE_DATE;
    else if ((hints & Qt::ImhDate) && (hints & Qt::ImhTime))
        purpose = ZWP_TEXT_INPUT_V4_CONTENT_PURPOSE_DATETIME;
    else if ((hints & Qt::ImhDate) == 0 && (hints & Qt::ImhTime))
        purpose = ZWP_TEXT_INPUT_V4_CONTENT_PURPOSE_TIME;
    if (hints & Qt::ImhPreferLatin)
        hint |= ZWP_TEXT_INPUT_V4_CONTENT_HINT_LATIN;
    if (hints & Qt::ImhMultiLine)
        hint |= ZWP_TEXT_INPUT_V4_CONTENT_HINT_MULTILINE;
    if (hints & Qt::ImhDigitsOnly)
        purpose = ZWP_TEXT_INPUT_V4_CONTENT_PURPOSE_DIGITS;
    if (hints & Qt::ImhFormattedNumbersOnly)
        purpose = ZWP_TEXT_INPUT_V4_CONTENT_PURPOSE_NUMBER;
    if (hints & Qt::ImhUppercaseOnly)
        hint |= ZWP_TEXT_INPUT_V4_CONTENT_HINT_UPPERCASE;
    if (hints & Qt::ImhLowercaseOnly)
        hint |= ZWP_TEXT_INPUT_V4_CONTENT_HINT_LOWERCASE;
    if (hints & Qt::ImhDialableCharactersOnly)
        purpose = ZWP_TEXT_INPUT_V4_CONTENT_PURPOSE_PHONE;
    if (hints & Qt::ImhEmailCharactersOnly)
       purpose = ZWP_TEXT_INPUT_V4_CONTENT_PURPOSE_EMAIL;
    if (hints & Qt::ImhUrlCharactersOnly)
       purpose = ZWP_TEXT_INPUT_V4_CONTENT_PURPOSE_URL;
    if (hints & Qt::ImhLatinOnly)
        hint |= ZWP_TEXT_INPUT_V4_CONTENT_HINT_LATIN;

    return WaylandInputMethodContentType{hint, purpose};
}

int WaylandInputMethodEventBuilder::indexFromWayland(const QString &text, int length, int base)
{
    if (length == 0)
        return base;

    if (length < 0) {
        const QByteArray &utf8 = QStringView{text}.left(base).toUtf8();
        return QString::fromUtf8(utf8.left(qMax(utf8.length() + length, 0))).length();
    } else {
        const QByteArray &utf8 = QStringView{text}.mid(base).toUtf8();
        return QString::fromUtf8(utf8.left(length)).length() + base;
    }
}

int WaylandInputMethodEventBuilder::trimmedIndexFromWayland(const QString &text, int length, int base)
{
    if (length == 0)
        return base;

    if (length < 0) {
        const QByteArray &utf8 = QStringView{text}.left(base).toUtf8();
        const int len = utf8.size();
        const int start = len + length;
        if (start <= 0)
            return 0;

        for (int i = 0; i < 4; i++) {
            if (start + i >= len)
                return base;

            const unsigned char ch = utf8.at(start + i);
            // check if current character is a utf8's initial character.
            if (ch < 0x80 || ch > 0xbf)
                return QString::fromUtf8(utf8.left(start + i)).length();
        }
    } else {
        const QByteArray &utf8 = QStringView{text}.mid(base).toUtf8();
        const int len = utf8.size();
        const int start = length;
        if (start >= len)
            return base + QString::fromUtf8(utf8).length();

        for (int i = 0; i < 4; i++) {
            const unsigned char ch = utf8.at(start - i);
            // check if current character is a utf8's initial character.
            if (ch < 0x80 || ch > 0xbf)
                return base + QString::fromUtf8(utf8.left(start - i)).length();
        }
    }
    return -1;
}

int WaylandInputMethodEventBuilder::indexToWayland(const QString &text, int length, int base)
{
    return QStringView{text}.mid(base, length).toUtf8().size();
}

} // namespace Compositor

} // namespace Aurora

