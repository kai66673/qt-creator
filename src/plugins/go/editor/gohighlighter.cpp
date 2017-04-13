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

static const TextEditor::TextStyle TEXT_STYLES[] = {
    TextEditor::C_TEXT,
    TextEditor::C_NUMBER,
    TextEditor::C_STRING,
    TextEditor::C_TYPE,
    TextEditor::C_FIELD,
    TextEditor::C_VIRTUAL_METHOD,
    TextEditor::C_FUNCTION,
    TextEditor::C_KEYWORD,
    TextEditor::C_PRIMITIVE_TYPE,
    TextEditor::C_OPERATOR,
    TextEditor::C_COMMENT,
    TextEditor::C_DOXYGEN_COMMENT,
    TextEditor::C_DOXYGEN_TAG,
    TextEditor::C_VISUAL_WHITESPACE,
};


using namespace TextEditor;

namespace GoEditor {
namespace Internal {

GoHighlighter::GoHighlighter(QTextDocument *parent) :
    SyntaxHighlighter(parent)
{ init(); }

GoHighlighter::GoHighlighter(TextDocument *parent) :
    SyntaxHighlighter(parent)
{ init(); }

GoHighlighter::~GoHighlighter()
{ }

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
    GoLexer *lexer = GoLexer::instance();
    QList<GoToken> tokens = lexer->tokenize(text, previousBlockState());
    LexerState currentState = lexer->lexerState();

    // Set format for tokens
    for (int i = 0; i < tokens.count(); ++i) {
        const GoToken &tk = tokens.at(i);
        const bool isFirst = (i == 0);
        const bool isLast = (i == tokens.count() - 1);
        switch (tk.kind()) {
        case T_LBRACE:
        case T_LBRACKET:
        case T_LPAREN:
            ++braceDepth;
            if (isFirst)
                TextDocumentLayout::userData(currentBlock())->setFoldingStartIncluded(true);
            parentheses.push_back(Parenthesis(Parenthesis::Opened, tk.ch(), tk.start()));
            break;
        case T_RBRACE:
        case T_RBRACKET:
        case T_RPAREN:
            --braceDepth;
            if (isLast)
                TextDocumentLayout::userData(currentBlock())->setFoldingEndIncluded(true);
            else
                foldingIndent = qMin(braceDepth, foldingIndent);
            parentheses.push_back(Parenthesis(Parenthesis::Closed, tk.ch(), tk.start()));
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
    const QTextCharFormat whiteSpaceFormat = formatForCategory(styleCategory(TextEditor::C_VISUAL_WHITESPACE));
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
    QVector<TextEditor::TextStyle> categories;
    for (size_t i = 0; i < sizeof(TEXT_STYLES)/sizeof(TextEditor::TextStyle); ++i)
        categories << TEXT_STYLES[i];
    setTextFormatCategories(categories);
}

int GoHighlighter::styleCategory(TextEditor::TextStyle style)
{
    static QHash<TextEditor::TextStyle, int> categories; // static for better performance
    if (categories.isEmpty())
        for (size_t i = 0; i < sizeof(TEXT_STYLES)/sizeof(TextEditor::TextStyle); ++i)
            categories[TEXT_STYLES[i]] = i;

    return categories[style];
}

int GoHighlighter::tokenCategory(const GoToken &tk)
{
    if (tk.isKeyword())
        return styleCategory(TextEditor::C_KEYWORD);
    else if (tk.isPrimitive())
        return styleCategory(TextEditor::C_PRIMITIVE_TYPE);
    else if (tk.isOperator())
        return styleCategory(TextEditor::C_OPERATOR);
    else if (tk.is(T_STRING) || tk.is(T_RUNE))
        return styleCategory(TextEditor::C_STRING);
    else if (tk.is(T_COMMENT))
        return styleCategory(TextEditor::C_COMMENT);
    else if (tk.is(T_NUMBER))
        return styleCategory(TextEditor::C_NUMBER);

    return -1;
}

} // namespace Internal
} // namespace GoEditor
