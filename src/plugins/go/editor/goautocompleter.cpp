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

#include "goautocompleter.h"
#include "golexer.h"

#include <QtDebug>

namespace GoEditor {
namespace Internal {

static bool shouldInsertMatchingText(QChar lookAhead)
{
    switch (lookAhead.unicode()) {
    case '{': case '}':
    case ']': case ')':
    case ';': case ',':
        return true;

    default:
        if (lookAhead.isSpace())
            return true;

        return false;
    } // switch

    return false;
}

GoAutoCompleter::GoAutoCompleter() :
    TextEditor::AutoCompleter()
{ }

bool GoAutoCompleter::contextAllowsAutoBrackets(const QTextCursor & cursor,
                                                const QString & textToInsert) const
{
    if (!shouldInsertMatchingText(cursor.document()->characterAt(cursor.selectionEnd())))
        return false;

    if (textToInsert.length() == 1) {
        QChar ch;
        ch = textToInsert.at(0);
        if (ch == QLatin1Char('"') || ch == QLatin1Char('\''))
            return !isInCommentStrict(cursor);
    }

    return !isInComment(cursor);
}
bool GoAutoCompleter::contextAllowsElectricCharacters(const QTextCursor & /*cursor*/) const
{ return true; }

bool GoAutoCompleter::isInComment(const QTextCursor &cursor) const
{ return GoLexer::tokenUnderCursorIsLiteralOrComment(cursor); }

QString GoAutoCompleter::insertMatchingBrace(const QTextCursor &cursor, const QString &text, QChar la, bool skipChars, int *skippedChars) const
{
    if (text.length() == 1) {
        switch (text.at(0).toLatin1()) {
            case '{': return QStringLiteral("}");
            case '[': return QStringLiteral("]");
            case '(': return QStringLiteral(")");
            case '"': return QStringLiteral("\"");
            case '\'': return QStringLiteral("\'");
        }

        return QString();
    }

    return TextEditor::AutoCompleter::insertMatchingBrace(cursor, text, la, skipChars, skippedChars);
}

bool GoAutoCompleter::isInCommentStrict(const QTextCursor &cursor) const
{ return GoLexer::tokenUnderCursorIsComment(cursor); }

} // namespace Internal
} // namespace GoEditor
