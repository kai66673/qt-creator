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

#include "gohighlighter.h"
#include "golexer.h"
#include "gotoken.h"

#include <utils/qtcassert.h>

using namespace TextEditor;

namespace GoEditor {
namespace Internal {

enum Format {
    Format_Number,
    Format_String,
    Format_Keyword,
    Format_PrimitiveType,
    Format_Operator,
    Format_Comment,
    Format_VisualWhitespace,
    Format_Count
};

static TextEditor::TextStyle styleForFormat(int format)
{
    using namespace TextEditor;
    const auto f = Format(format);
    switch (f) {
        case Format_Number: return C_NUMBER;
        case Format_String: return C_STRING;
        case Format_Keyword: return C_KEYWORD;
        case Format_PrimitiveType: return C_PRIMITIVE_TYPE;
        case Format_Operator: return C_OPERATOR;
        case Format_Comment: return C_COMMENT;
        case Format_VisualWhitespace: return C_VISUAL_WHITESPACE;
        case Format_Count:
            QTC_CHECK(false); // should never get here
            return C_TEXT;
    }

    QTC_CHECK(false); // should never get here
    return C_TEXT;
}

GoHighlighter::GoHighlighter(QTextDocument *parent) :
    SyntaxHighlighter(parent)
{ init(); }

GoHighlighter::GoHighlighter(TextDocument *parent) :
    SyntaxHighlighter(parent)
{ init(); }

GoHighlighter::~GoHighlighter()
{ }

static QLatin1Char parenthesisChar(TokenKind kind)
{
    switch (kind) {
        case T_LBRACE: return QLatin1Char('{');
        case T_LBRACKET: return QLatin1Char('[');
        case T_LPAREN: return QLatin1Char('(');
        case T_RBRACE: return QLatin1Char('}');
        case T_RBRACKET: return QLatin1Char(']');
        case T_RPAREN: return QLatin1Char(')');
        default: return QLatin1Char('\0');
    }
    return QLatin1Char('\0');
}

void GoHighlighter::highlightBlock(const QString &text)
{
    // Reset to defaults
    TextEditor::TextBlockUserData *userData = TextEditor::TextDocumentLayout::testUserData(currentBlock());
    if (userData) {
        userData->setFoldingIndent(0);
        userData->setFoldingStartIncluded(false);
        userData->setFoldingEndIncluded(false);
    }

    Parentheses parentheses;
    parentheses.clear();

    LexerState previousState(previousBlockState());
    int foldingIndent = previousState.braceDepth();
    int braceDepth = previousState.braceDepth();
    bool inMultiLineComment = previousState.multiLine() == LexerState::MultiLineComment;

    // Parse current text line
    GoLexer lexer;
    QList<GoToken> tokens = lexer.tokenize(text, previousBlockState());
    LexerState currentState = lexer.lexerState();

    // Set format for tokens
    for (int i = 0; i < tokens.count(); ++i) {
        const GoToken &tk = tokens.at(i);
        const bool isFirst = (i == 0);
        const bool isLast = (i == tokens.count() - 1);
        const TokenKind kind = tk.kind();
        switch (kind) {
        case T_LBRACE:
        case T_LBRACKET:
        case T_LPAREN:
            ++braceDepth;
            if (isFirst)
                TextDocumentLayout::userData(currentBlock())->setFoldingStartIncluded(true);
            parentheses.push_back(Parenthesis(Parenthesis::Opened, parenthesisChar(kind), tk.start()));
            break;
        case T_RBRACE:
        case T_RBRACKET:
        case T_RPAREN:
            --braceDepth;
            if (isLast)
                TextDocumentLayout::userData(currentBlock())->setFoldingEndIncluded(true);
            else
                foldingIndent = qMin(braceDepth, foldingIndent);
            parentheses.push_back(Parenthesis(Parenthesis::Closed, parenthesisChar(kind), tk.start()));
            break;
        case T_COMMENT:
            if (inMultiLineComment && text.midRef(tk.end() - 2, 2) == QLatin1String("*/")) {
                --braceDepth;
                if (isLast)
                    TextDocumentLayout::userData(currentBlock())->setFoldingEndIncluded(true);
                else
                    foldingIndent = qMin(braceDepth, foldingIndent);
                parentheses.push_back(Parenthesis(Parenthesis::Closed, QLatin1Char('-'), tk.end() - 1));
                inMultiLineComment = false;

            } else if (!inMultiLineComment && isLast && currentState.multiLine() == LexerState::MultiLineComment) {
                ++braceDepth;
                if (isFirst)
                    TextDocumentLayout::userData(currentBlock())->setFoldingStartIncluded(true);
                parentheses.push_back(Parenthesis(Parenthesis::Opened, QLatin1Char('+'), tk.start()));
                inMultiLineComment = true;
            }
            break;
        default:
            break;
        }

        const int category = tokenCategory(tk);
        if (category >= 0)
            setFormat(tk.start(), tk.count(), formatForCategory(category));
    }

    // Visualize white space, if enabled
    const QTextCharFormat whiteSpaceFormat = formatForCategory(Format_VisualWhitespace);
    int previousTokenEnd = 0;
    for (int i = 0; i < tokens.count(); ++i) {
        const GoToken &tk = tokens.at(i);
        setFormat(previousTokenEnd, tk.start() - previousTokenEnd, whiteSpaceFormat);

        switch (tk.kind()) {
        case T_COMMENT:
        case T_STRING:
        case T_RUNE: {
            int s = tk.start(), e = tk.end();
            while (s < e) {
                const QChar ch = text.at(s);
                if (ch.isSpace()) {
                    const int start = s;
                    do {
                        ++s;
                    } while (s < e && text.at(s).isSpace());
                    setFormat(start, s - start, whiteSpaceFormat);
                } else {
                    ++s;
                }
            }
        } break;

        default:
            break;
        } // end of switch

        previousTokenEnd = tk.end();
    }

    setFormat(previousTokenEnd, text.length() - previousTokenEnd, whiteSpaceFormat);

    // Finalize
    currentState.setBraceDepth(braceDepth);
    setCurrentBlockState(currentState.state());

    TextEditor::TextDocumentLayout::setParentheses(currentBlock(), parentheses);
    TextEditor::TextDocumentLayout::setFoldingIndent(currentBlock(), foldingIndent);
}

void GoHighlighter::init()
{
    setTextFormatCategories(Format_Count, styleForFormat);
}

int GoHighlighter::tokenCategory(const GoToken &tk)
{
    if (tk.isKeyword())
        return Format_Keyword;
    else if (tk.isPrimitive())
        return Format_PrimitiveType;
    else if (tk.isOperator())
        return Format_Operator;
    else if (tk.is(T_STRING) || tk.is(T_RUNE))
        return Format_String;
    else if (tk.is(T_COMMENT))
        return Format_Comment;
    else if (tk.is(T_NUMBER))
        return Format_Number;

    return -1;
}

} // namespace Internal
} // namespace GoEditor
