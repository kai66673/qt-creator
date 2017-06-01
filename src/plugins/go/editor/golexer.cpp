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

#include "golexer.h"

#include <QList>
#include <QString>
#include <QtAlgorithms>

static QStringList s_builtings {
    "append",
    "bool",
    "break",
    "byte",
    "cap",
    "case",
    "chan",
    "close",
    "complex",
    "complex128",
    "complex64",
    "const",
    "continue",
    "copy",
    "default",
    "defer",
    "delete",
    "else",
    "error",
    "fallthrough",
    "false",
    "float32",
    "float64",
    "for",
    "func",
    "go",
    "goto",
    "if",
    "imag",
    "import",
    "int",
    "int16",
    "int32",
    "int64",
    "int8",
    "interface",
    "iota",
    "len",
    "make",
    "map",
    "new",
    "nil",
    "package",
    "panic",
    "print",
    "println",
    "range",
    "real",
    "recover",
    "return",
    "rune",
    "select",
    "string",
    "struct",
    "switch",
    "true",
    "type",
    "uint",
    "uint16",
    "uint32",
    "uint64",
    "uint8",
    "uintptr",
    "var",
};

static QSet<QString> s_keywords {
    "break",
    "case",
    "chan",
    "const",
    "continue",
    "default",
    "defer",
    "else",
    "fallthrough",
    "false",
    "for",
    "func",
    "go",
    "goto",
    "if",
    "import",
    "interface",
    "iota",
    "map",
    "nil",
    "package",
    "range",
    "return",
    "select",
    "struct",
    "switch",
    "true",
    "type",
    "var",
};

static QSet<QString> s_types {
    "bool",
    "byte",
    "complex64",
    "complex128",
    "error",
    "float32",
    "float64",
    "int",
    "int8",
    "int16",
    "int32",
    "int64",
    "rune",
    "string",
    "uint",
    "uintptr",
    "uint8",
    "uint16",
    "uint32",
    "uint64",
};

static QSet<QString> s_operators {
    "append",
    "cap",
    "close",
    "complex",
    "copy",
    "delete",
    "imag",
    "len",
    "make",
    "new",
    "panic",
    "print",
    "println",
    "real",
    "recover",
};

namespace GoEditor {
namespace Internal {

GoLexer::GoLexer() :
    m_lexerState(0),
    m_text(0),
    m_length(0),
    m_position(0)
{ }

QChar GoLexer::ch(int offset)
{
    if ((m_position + offset) < m_length)
        return m_text[m_position + offset];
    return QLatin1Char('\0');
}

QChar GoLexer::chpp()
{
    if (m_position < m_length)
        return m_text[m_position++];
    return QLatin1Char('\0');
}

GoToken GoLexer::parseDefault()
{
    QChar c = chpp();
    GoToken token;

    if (c.isSpace()) // White space
        skipWhiteSpace();
    else if (c.isLetter() || c == QLatin1Char('_')) // Identifier
        token = parseIdentifier();
    else if (c.isDigit()) // Number
        token = parseNumber(c);
    else {
        switch (c.toLatin1()) {
        // Strings & runes
        case '"':
            token = parseString();
            break;
        case '`':
            m_lexerState.setMultiLine(LexerState::MultiLineRawString);
            token = parseString();
            break;
        case '\'':
            token = parseRune();
            break;

        // Comments
        case '/':
            switch (ch().toLatin1()) {
            case '/':
                token = parseSingleLineComment();
                break;
            case '*':
                m_lexerState.setMultiLine(LexerState::MultiLineComment);
                token = parseMultiLineComment();
                break;
            default:
                token = GoToken(m_tokenPosition, 1, T_SLASH);
            }
            break;

        // Operations
        case '+':
            if (ch() == QLatin1Char('='))
                token = GoToken(m_tokenPosition, 2, T_PLUS_EQUAL);
            else
                token = GoToken(m_tokenPosition, 1, T_PLUS);
            break;
        case '-':
            if (ch() == QLatin1Char('='))
                token = GoToken(m_tokenPosition, 2, T_MINUS_EQUAL);
            else if (ch() == QLatin1Char('>'))
                token = GoToken(m_tokenPosition, 2, T_RARROW);
            else
                token = GoToken(m_tokenPosition, 1, T_MINUS);
            break;
        case ':':
            if (ch() == QLatin1Char('='))
                token = GoToken(m_tokenPosition, 2, T_COLON_EQUAL);
            else
                token = GoToken(m_tokenPosition, 1, T_COLON);
            break;
        case '=':
            if (ch() == QLatin1Char('='))
                token = GoToken(m_tokenPosition, 2, T_EQUAL_EQUAL);
            else
                token = GoToken(m_tokenPosition, 1, T_EQUAL);
            break;
        case '.':
            if (ch().isDigit())
                parseNumber(c);
            else if (ch() == QLatin1Char('.') && ch(1) == QLatin1Char('.'))
                token = GoToken(m_tokenPosition, 3, T_DOT_DOT_DOT);
            else
                token = GoToken(m_tokenPosition, 1, T_DOT);
            break;
        case '<':
            if (ch() == QLatin1Char('<')) {
                if (ch(1) == QLatin1Char('='))
                    token = GoToken(m_tokenPosition, 3, T_LESS_LESS_EQUAL);
                else
                    token = GoToken(m_tokenPosition, 2, T_LESS_LESS);
            } else if (ch() == QLatin1Char('=')) {
                token = GoToken(m_tokenPosition, 2, T_LESS_EQUAL);
            } else if (ch() == QLatin1Char('-')) {
                token = GoToken(m_tokenPosition, 2, T_LARROW);
            } else {
                token = GoToken(m_tokenPosition, 1, T_LESS);
            }
            break;
        case '>':
            if (ch() == QLatin1Char('>')) {
                if (ch(1) == QLatin1Char('='))
                    token = GoToken(m_tokenPosition, 3, T_GREATER_GREATER_EQUAL);
                else
                    token = GoToken(m_tokenPosition, 2, T_GREATER_GREATER);
            } else if (ch() == QLatin1Char('=')) {
                token = GoToken(m_tokenPosition, 2, T_GREATER_EQUAL);
            } else {
                token = GoToken(m_tokenPosition, 1, T_GREATER);
            }
            break;
        case '!':
            if (ch() == QLatin1Char('='))
                token = GoToken(m_tokenPosition, 2, T_EXCLAM_EQUAL);
            else
                token = GoToken(m_tokenPosition, 1, T_EXCLAM);
            break;
        case '|':
            if (ch() == QLatin1Char('='))
                token = GoToken(m_tokenPosition, 2, T_PIPE_EQUAL);
            else if (ch() == QLatin1Char('|'))
                token = GoToken(m_tokenPosition, 1, T_PIPE_PIPE);
            else
                token = GoToken(m_tokenPosition, 1, T_PIPE);
            break;
        case '*':
            if (ch() == QLatin1Char('='))
                token = GoToken(m_tokenPosition, 2, T_ASTERISK_EQUAL);
            else
                token = GoToken(m_tokenPosition, 1, T_ASTERISK);
            break;
        case '&':
            if (ch() == QLatin1Char('^')) {
                if (ch(1) == QLatin1Char('='))
                    token = GoToken(m_tokenPosition, 3, T_AMPER_CARET_EQUAL);
                else
                    token = GoToken(m_tokenPosition, 2, T_AMPER_CARET);
            } else if (ch() == QLatin1Char('&')) {
                token = GoToken(m_tokenPosition, 2, T_AMPER_AMPER);
            } else {
                token = GoToken(m_tokenPosition, 1, T_AMPER);
            }
            break;
        case ',':
            token = GoToken(m_tokenPosition, 1, T_COMMA);
            break;

        // Misc

        // Delimiters
        case ';':
            token = GoToken(m_tokenPosition, 1, T_SEMICOLON);
            break;
        case '{':
            token = GoToken(m_tokenPosition, 1, T_LBRACE, m_text);
            break;
        case '}':
            token = GoToken(m_tokenPosition, 1, T_RBRACE, m_text);
            break;
        case '[':
            token = GoToken(m_tokenPosition, 1, T_LBRACKET, m_text);
            break;
        case ']':
            token = GoToken(m_tokenPosition, 1, T_RBRACKET, m_text);
            break;
        case '(':
            token = GoToken(m_tokenPosition, 1, T_LPAREN, m_text);
            break;
        case ')':
            token = GoToken(m_tokenPosition, 1, T_RPAREN, m_text);
            break;
        default:
            break;
        }
    }

    return token;
}

GoToken GoLexer::parseIdentifier()
{
    QChar c;
    while (m_position < m_length) {
        c = ch();
        if (!c.isLetterOrNumber() && c != QLatin1Char('_'))
            break;
        ++m_position;
    }

    const int tokenStart = m_tokenPosition;
    const int tokenCount = m_position - m_tokenPosition;
    QString identifier(&m_text[tokenStart], tokenCount);

    TokenKind kind = T_IDENTIFIER;
    if (s_keywords.contains(identifier))
        kind = T_KEYWORD;
    else if (s_types.contains(identifier))
        kind = T_PRIMITIVE;
    else if (s_operators.contains(identifier))
        kind = T_OPERATOR;

    return GoToken(tokenStart, tokenCount, kind, m_text);
}

GoToken GoLexer::parseMultiLineComment()
{
    QChar c;
    bool expectCommentClosing = false;
    while (m_position < m_length) {
        c = chpp();

        if (expectCommentClosing && c == QLatin1Char('/')) { // Multiline comment ended
            m_lexerState.setMultiLine(LexerState::NoMultiLine);
            break;
        }

        if (c == QLatin1Char('*'))
            expectCommentClosing = true;
        else
            expectCommentClosing = false;
    }
    return GoToken(m_tokenPosition, m_position - m_tokenPosition, T_COMMENT);
}

GoToken GoLexer::parseRune()
{
    QChar c;
    while (m_position < m_length) {
        c = chpp();
        if (c == QLatin1Char('\''))
            break;
    }

    return GoToken(m_tokenPosition, m_position - m_tokenPosition, T_RUNE);
}

GoToken GoLexer::parseSingleLineComment()
{
    QChar c;
    while (m_position < m_length) {
        c = chpp();
        if (c == QLatin1Char('\n'))
            break;
    }
    return GoToken(m_tokenPosition, m_position - m_tokenPosition, T_COMMENT);
}

GoToken GoLexer::parseNumber(QChar first)
{
    enum NumberFlag {
        FixedPointType = 0x1,
        FloatPointType = 0x2,
        HexType = 0x4,
        OctalType = 0x8,
    };
    int flags;
    switch (first.toLatin1()) {
    case '0':
        if (ch() == QLatin1Char('x') || ch() == QLatin1Char('X')) {
            flags = HexType;
            ++m_position;
        } else if (ch() == QLatin1Char('.')) {
            flags = FloatPointType;
            ++m_position;
        } else {
            flags = OctalType | FloatPointType;
        }
        break;
    case '.':
        flags = FloatPointType;
        break;
    default:
        flags = FixedPointType | FloatPointType;
        break;
    }

    QChar c;
    while (m_position < m_length) {
        c = ch();

        if (c == QLatin1Char('.'))
            flags = FloatPointType;

        if ((flags & HexType) && !QString(QLatin1String("0123456789abcdefABCDEF")).contains(c))
            break;
        if ((flags & FloatPointType) && !QString(QLatin1String("0123456789eEi")).contains(c))
            break;
        if ((flags & FixedPointType) && !c.isDigit() && c != QLatin1Char('i'))
            break;
        ++m_position;
    }

    return GoToken(m_tokenPosition, m_position - m_tokenPosition, T_NUMBER);
}

GoToken GoLexer::parseString()
{
    const LexerState::MultiLine ml = m_lexerState.multiLine();
    const QChar quoteChar = (ml == LexerState::MultiLineRawString) ? QLatin1Char('`') : QLatin1Char('"');

    QChar c, prev;
    while (m_position < m_length) {
        prev = m_text[m_position - 1];
        c = chpp();
        if (c == QLatin1Char('\\')) {
            // ignoring escaped symbol
            chpp();
            continue;
        }
        if (c == quoteChar) {
            m_lexerState.setMultiLine(LexerState::NoMultiLine);
            break;
        }
    }

    return GoToken(m_tokenPosition, m_position - m_tokenPosition, T_STRING);
}

void GoLexer::skipWhiteSpace()
{
    QChar c;
    while (m_position < m_length) {
        c = ch();
        if (!c.isSpace())
            break;
        ++m_position;
    }
}

QList<GoToken> GoLexer::tokenize(const QString &text, int prevBlockState)
{
    QList<GoToken> tokens;

    m_text = text.constData();
    m_tokenPosition = m_position = 0;
    m_length = text.length();
    if (prevBlockState < 0)
        prevBlockState = 0;
    m_lexerState.setState(prevBlockState);
    GoToken token;

    while (m_position < m_length) {
        switch (m_lexerState.multiLine()) {
        case LexerState::MultiLineRawString:
            token = parseString();
            break;
        case LexerState::MultiLineComment:
            skipWhiteSpace();
            m_tokenPosition = m_position; // Start position should point first non-space character
            token = parseMultiLineComment();
            break;
        default:
            token = parseDefault();
            break;
        }
        if (token.isValid())
            tokens.append(token);
        m_tokenPosition = m_position;
    }

    return tokens;
}

QList<GoToken> GoLexer::tokenize(const QTextBlock &block)
{
    const int prevState = block.previous().isValid() ? block.previous().userState() : -1;
    return tokenize(block.text(), prevState);
}

int GoLexer::tokenBefore(const QList<GoToken> &tokens, int offset)
{
    for (int index = tokens.size() - 1; index >= 0; --index) {
        const GoToken &tk = tokens.at(index);
        if (tk.start() <= offset + 1)
            return index;
    }

    return -1;
}

TokenKind GoLexer::tokenKindUnderCursor(const QTextCursor &cursor)
{
    GoLexer lexer;

    const QList<GoToken> tokens = lexer.tokenize(cursor.block());
    const int tokenIdx = GoLexer::tokenBefore(tokens, qMax(0, cursor.positionInBlock() - 1));
    const GoToken &tk = (tokenIdx == -1) ? GoToken() : tokens.at(tokenIdx);

    return tk.kind();
}

bool GoLexer::tokenUnderCursorIsLiteralOrComment(const QTextCursor &cursor)
{
    TokenKind kind = tokenKindUnderCursor(cursor);
    switch (kind) {
        case T_COMMENT: case T_NUMBER: case T_STRING: case T_RUNE:
            return true;
        default:
            return false;
    }

    return false;
}

bool GoLexer::tokenUnderCursorIsComment(const QTextCursor &cursor)
{ return tokenKindUnderCursor(cursor) == T_COMMENT; }

QStringList &GoLexer::builtins()
{ return s_builtings; }

} // namespace Internal
} // namespace GoEditor
