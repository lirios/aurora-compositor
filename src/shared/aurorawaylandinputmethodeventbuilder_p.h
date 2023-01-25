// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef AURORA_COMPOSITOR_WAYLANDINPUTMETHODEVENTBUILDER_H
#define AURORA_COMPOSITOR_WAYLANDINPUTMETHODEVENTBUILDER_H

#include <QInputMethodEvent>

namespace Aurora {

namespace Compositor {

class WaylandInputMethodEventBuilder
{
public:
    WaylandInputMethodEventBuilder() = default;
    ~WaylandInputMethodEventBuilder();

    void reset();

    void setCursorPosition(int32_t index, int32_t anchor);
    void setDeleteSurroundingText(uint32_t beforeLength, uint32_t afterLength);

    void addPreeditStyling(uint32_t index, uint32_t length, uint32_t style);
    void setPreeditCursor(int32_t index);

    QInputMethodEvent *buildCommit(const QString &text);
    QInputMethodEvent *buildPreedit(const QString &text);

    static int indexFromWayland(const QString &text, int length, int base = 0);
    static int indexToWayland(const QString &text, int length, int base = 0);

    static int trimmedIndexFromWayland(const QString &text, int length, int base = 0);
private:
    QPair<int, int> replacementForDeleteSurrounding();

    int32_t m_anchor = 0;
    int32_t m_cursor = 0;
    uint32_t m_deleteBefore = 0;
    uint32_t m_deleteAfter = 0;

    int32_t m_preeditCursor = 0;
    QList<QInputMethodEvent::Attribute> m_preeditStyles;
};

struct WaylandInputMethodContentType {
    uint32_t hint = 0;
    uint32_t purpose = 0;

    static WaylandInputMethodContentType convert(Qt::InputMethodHints hints);
    static WaylandInputMethodContentType convertV4(Qt::InputMethodHints hints);
};


} // namespace Compositor

} // namespace Aurora

#endif // AURORA_COMPOSITOR_WAYLANDINPUTMETHODEVENTBUILDER_H
