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

#include "gosnippetprovider.h"
#include "goconstants.h"
#include "gohighlighter.h"
#include "goindenter.h"
#include "goautocompleter.h"

#include <texteditor/snippets/snippeteditor.h>
#include <texteditor/textdocument.h>

#include <QCoreApplication>

namespace GoEditor {
namespace Internal {

GoSnippetProvider::GoSnippetProvider()
{ }

GoSnippetProvider::~GoSnippetProvider()
{ }

QString GoSnippetProvider::groupId() const
{
    return QLatin1String(Go::Constants::GO_SNIPPETS_GROUP_ID);
}

QString GoSnippetProvider::displayName() const
{
    return QCoreApplication::translate("GoEditor::GoSnippetProvider", "GO");
}

void GoSnippetProvider::decorateEditor(TextEditor::SnippetEditorWidget *editor) const
{
    editor->textDocument()->setSyntaxHighlighter(new GoHighlighter);
    editor->textDocument()->setIndenter(new GoIndenter);
    editor->setAutoCompleter(new GoAutoCompleter);
}

}   // namespace Internal
}   // namespace GoEditor
