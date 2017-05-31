/****************************************************************************
**
** Copyright (C) 2016 Alexander Kudryavtsev.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
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
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once

#include "gotoken.h"

#include <texteditor/texteditorconstants.h>

#include <QSet>
#include <QTextBlock>

namespace GoEditor {
namespace Internal {

class LexerState {
public:
    enum MultiLine {
        NoMultiLine = 0,
        MultiLineRawString,
        MultiLineComment
    };
    LexerState(int state){ m_state = state < 0 ? 0 : state; }
    int state() const { return m_state; }
    void setState(int state) { m_state = state; }

    MultiLine multiLine() const { return m_fields.multiLine; }
    void setMultiLine(MultiLine multiLine) { m_fields.multiLine = multiLine; }

    int braceDepth() const { return m_fields.braceDepth; }
    void setBraceDepth(int braceDepth) { m_fields.braceDepth = braceDepth; }

private:
    struct Fields {
        MultiLine multiLine : 4;
        int braceDepth      : 8;
    };
    union {
        Fields m_fields;
        int m_state;
    };
};

class GoLexer
{
public:
    GoLexer();

    QList<GoToken> tokenize(const QString &text, int prevBlockState);
    QList<GoToken> tokenize(const QTextBlock &block);
    LexerState lexerState() const { return m_lexerState; }
    static int tokenBefore(const QList<GoToken> &tokens, int offset);
    static TokenKind tokenKindUnderCursor(const QTextCursor &cursor);
    static bool tokenUnderCursorIsLiteralOrComment(const QTextCursor &cursor);
    static bool tokenUnderCursorIsComment(const QTextCursor &cursor);

    static QStringList &builtins();

private:
    QChar ch(int offset = 0); // returns current char
    QChar chpp(); // returns current char with position increment

    void skipWhiteSpace();
    GoToken parseDefault();
    GoToken parseString();
    GoToken parseRune();
    GoToken parseSingleLineComment();
    GoToken parseMultiLineComment();
    GoToken parseIdentifier();
    GoToken parseNumber(QChar first);

private:
    LexerState m_lexerState;
    const QChar *m_text;
    int m_length;
    int m_tokenPosition;
    int m_position;
};

} // namespace Internal
} // namespace GoEditor

