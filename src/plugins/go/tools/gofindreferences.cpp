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
#include "findreferences.h"
#include "gofindreferences.h"
#include "gopackage.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/find/searchresultwindow.h>
#include <texteditor/basefilefind.h>

#include <QDir>

namespace GoTools {

GoFindReferences::GoFindReferences(QObject *parent)
    : QObject(parent)
{ }

void GoFindReferences::findReferences(GoSource::Ptr source, int pos, bool isReplace)
{
    FindReferences refs(source);
    if (Core::SearchResult *search = refs.proceedReferences(pos, isReplace)) {
        connect(search, &Core::SearchResult::activated,
                this, &GoFindReferences::openEditor);
        if (isReplace)
            connect(search, &Core::SearchResult::replaceButtonClicked,
                    this, &GoFindReferences::onReplaceButtonClicked);
        Core::SearchResultWindow::instance()->popup(Core::IOutputPane::ModeSwitch | Core::IOutputPane::WithFocus);
    }
}

void GoFindReferences::openEditor(const Core::SearchResultItem &item)
{
    if (item.path.size() > 0) {
        Core::EditorManager::openEditorAt(QDir::fromNativeSeparators(item.path.first()),
                                    item.mainRange.begin.line,
                                    item.mainRange.begin.column);
    } else {
        Core::EditorManager::openEditor(QDir::fromNativeSeparators(item.text));
    }
}

void GoFindReferences::onReplaceButtonClicked(const QString &text, const QList<Core::SearchResultItem> &items, bool preserveCase)
{
    if (!items.isEmpty()) {
        Core::SearchResult *search = qobject_cast<Core::SearchResult *>(sender());
        GoFindReferencesParameters parameters = search->userData().value<GoFindReferencesParameters>();
        const Core::SearchResultItem &firstItem = items.first();
        if (parameters.customReplaceSuffixForFirstItem.isEmpty() ||
            firstItem.text[firstItem.mainRange.begin.column] != QChar('\"')) {
            GoPackageCache::instance()->indexGoFiles(QString(), QSet<QString>(),
                                                     TextEditor::BaseFileFind::replaceAll(text, items, preserveCase).toSet());
        } else {
            QList<Core::SearchResultItem> head;
            head << items.first();
            QList<Core::SearchResultItem> rest = items;
            rest.removeFirst();
            TextEditor::BaseFileFind::replaceAll(text, rest, preserveCase);
            TextEditor::BaseFileFind::replaceAll(text + parameters.customReplaceSuffixForFirstItem, head, preserveCase);
        }
    }
}

}   // namespace GoTools
