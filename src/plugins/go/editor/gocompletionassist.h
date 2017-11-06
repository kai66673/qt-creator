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

#include "godocument.h"
#include "gosource.h"

#include <texteditor/codeassist/assistinterface.h>
#include <texteditor/codeassist/completionassistprovider.h>
#include <texteditor/codeassist/iassistprocessor.h>
#include <texteditor/codeassist/iassistproposal.h>
#include <texteditor/snippets/snippetassistcollector.h>


namespace GoEditor {
namespace Internal {

class GoCompletionAssistInterface;

class GoCompletionAssistProvider : public TextEditor::CompletionAssistProvider
{
    Q_OBJECT

public:
    GoCompletionAssistProvider(QObject *parent = 0);

    TextEditor::IAssistProcessor *createProcessor() const override;
    int activationCharSequenceLength() const override;
    bool isActivationCharSequence(const QString &sequence) const override;
};

class GoCompletionAssistProcessor: public TextEditor::IAssistProcessor
{
public:
    GoCompletionAssistProcessor();
    virtual ~GoCompletionAssistProcessor();

    virtual TextEditor::IAssistProposal *perform(const TextEditor::AssistInterface *interface_) override;

private:
    void addImports();

    TextEditor::SnippetAssistCollector m_snippetCollector;
    QScopedPointer<const GoCompletionAssistInterface> m_interface;
    QList<TextEditor::AssistProposalItemInterface *> m_completions;
};

class GoCompletionAssistInterface: public TextEditor::AssistInterface
{
public:
    GoCompletionAssistInterface(const QString &filePath,
                                QTextDocument *textDocument,
                                int position,
                                TextEditor::AssistReason reason,
                                GoDocument *doc);

    GoTools::GoSource::Ptr source() const;
    GoTools::GoSource::Ptr actualSource() const;

private:
    GoDocument *m_doc;
    unsigned m_revision;
    GoTools::GoSource::Ptr m_source;
};

}   // namespace Internal
}   // namespace GoEditor

