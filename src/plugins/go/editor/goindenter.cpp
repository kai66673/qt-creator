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

#include "goindenter.h"
#include "gocodeformatter.h"

#include <texteditor/tabsettings.h>

namespace GoEditor {
namespace Internal {

GoIndenter::GoIndenter(QTextDocument *doc)
    : TextEditor::TextIndenter(doc)
{ }

void GoIndenter::indentBlock(const QTextBlock &block,
                             const QChar &typedChar, const TextEditor::TabSettings &tabSettings, int cursorPositionInEditor)
{
    GoCodeFormatter codeFormatter(tabSettings);

    codeFormatter.updateStateUntil(block);
    int indentation;
    int padding;
    codeFormatter.calcIndentation(block, &indentation, &padding);

    if (isElectricCharacter(typedChar)) {
        int defaultIndentation;
        int defaultPadding;
        codeFormatter.calcIndentation(block, &defaultIndentation, &defaultPadding, true);
    }

    tabSettings.indentLine(block, indentation + padding, padding);
}

bool GoIndenter::isElectricCharacter(const QChar &ch) const
{
    switch (ch.toLatin1()) {
        case '{':
        case '}':
        case '(':
        case ')':
        case '=': // :=
        case ':': // "case ... :" or "Label:"
            return true;
    }

    return false;
}

} // namespace Internal
} // namespace GoEditor
