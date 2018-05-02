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

#include "gosource.h"
#include "goeditoroutline.h"

#include <texteditor/texteditor.h>

namespace GoEditor {
namespace Internal {

class GoEditorWidgetPrivate;

class GoEditor : public TextEditor::BaseTextEditor
{
    Q_OBJECT

public:
    GoEditor();

    static void decorateEditor(TextEditor::TextEditorWidget *editor);
};

class GoEditorFactory : public TextEditor::TextEditorFactory
{
    Q_OBJECT

public:
    GoEditorFactory();
};

class GoEditorWidget : public TextEditor::TextEditorWidget
{
    Q_OBJECT

public:
    GoEditorWidget();
    ~GoEditorWidget() override;

    GoTools::GoEditorOutline *outline() const;

    TextEditor::AssistInterface *createAssistInterface(TextEditor::AssistKind kind,
                                                       TextEditor::AssistReason reason) const override;

    QString evaluateIdentifierTypeDescription(int pos);
    void findUsages();
    void renameSymbolUnderCursor();

protected:
    virtual Utils::Link findLinkAt(const QTextCursor &tc, bool resolveTarget = true,
                                   bool inNextSplit = false) override;

private:
    void finalizeInitialization() override;
    void finalizeInitializationAfterDuplication(TextEditorWidget *other) override;
    void onGoDocumentUpdated(GoTools::GoSource::Ptr doc);

private:
    QScopedPointer<GoEditorWidgetPrivate> d;
};

} // namespace Internal
} // namespace GoEditor
