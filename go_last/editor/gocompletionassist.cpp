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
#include "gocompletionassist.h"
#include "goeditorconstants.h"
#include "goconstants.h"
#include "gocompletionassistvisitor.h"
#include "goiconprovider.h"
#include "golexer.h"
#include "gocodemodelmanager.h"
#include "gofunctionhintassistvisitor.h"

#include <coreplugin/id.h>
#include <texteditor/codeassist/assistproposalitem.h>
#include <texteditor/codeassist/functionhintproposal.h>
#include <texteditor/codeassist/genericproposal.h>
#include <texteditor/codeassist/genericproposalmodel.h>
#include <texteditor/codeassist/ifunctionhintproposalmodel.h>

namespace GoEditor {
namespace Internal {

class GoFunctionHintModel: public TextEditor::IFunctionHintProposalModel {
public:
    GoFunctionHintModel(const QStringList &functionArgs)
        : m_functionArgs(functionArgs)
        , m_currentArg(-1)
        , m_lastArgIsEllipsis(false)
    {
        if (m_functionArgs.last().contains(QLatin1String("...")))
            m_lastArgIsEllipsis = true;
    }

    void reset() override { }
    int size() const override { return 1; }

    QString text(int) const override {
        int currArg = m_currentArg == -1 ? 0 : m_currentArg;
        QString hintText = QLatin1String("(");
        int index = 0;
        for (const QString &arg: m_functionArgs) {
            bool markArg = currArg == index || (index == m_functionArgs.size() - 1 && m_lastArgIsEllipsis && currArg > index);
            if (markArg)
                hintText += QLatin1String("<b>");
            hintText += arg.toHtmlEscaped();
            if (markArg)
                hintText += QLatin1String("</b>");
            hintText += QLatin1String(", ");
            index++;
        }
        hintText.chop(2);
        hintText += QLatin1String(")");

        return hintText;
    }

    int activeArgument(const QString &prefix) const override {
        int argnr = 0;
        int parcount = 0;
        GoLexer lexer;
        const QList<GoToken> tokens = lexer.tokenize(prefix, -1);
        for (int i = 0; i < tokens.count(); ++i) {
            const GoToken &tk = tokens.at(i);
            if (tk.is(T_LPAREN))
                ++parcount;
            else if (tk.is(T_RPAREN))
                --parcount;
            else if (!parcount && tk.is(T_COMMA))
                ++argnr;
        }

        if (parcount < 0)
            return -1;

        if (argnr != m_currentArg)
            m_currentArg = argnr;

        return argnr;
    }

private:
    QStringList m_functionArgs;
    mutable int m_currentArg;
    mutable bool m_lastArgIsEllipsis;
};

class GoWithImportAssistProposalItem final: public TextEditor::AssistProposalItem
{
public:
    GoWithImportAssistProposalItem(const QString &text, const QString &detail, const QIcon &icon, const QString &importText)
        : m_importText(importText)
    {
        setText(text);
        setDetail(detail);
        setIcon(icon);
    }

    void applyContextualContent(TextEditor::TextDocumentManipulatorInterface &manipulator, int basePosition) const override
    {
        QString searchImportText = manipulator.textAt(0, basePosition);
        TextEditor::AssistProposalItem::applyContextualContent(manipulator, basePosition);

        GoLexer lexer;
        const QList<GoToken> tokens = lexer.tokenize(searchImportText, -1);

        TokenKind prevKind = T_INVALID;
        int packageKeywordEnd = -1;
        int prevKeywordEnd = -1;
        for (const GoToken &tk : tokens) {
            TokenKind kind = tk.kind();
            if (kind == T_COMMENT)
                continue;
            if (kind == T_PACKAGE) {
                packageKeywordEnd = tk.end();
            } else if (prevKind == T_IMPORT && kind == T_LPAREN) {
                manipulator.replace(tk.end(), 0, QStringLiteral("\n\t\"") + m_importText + QLatin1Char('\"'));
                return;
            } else if (prevKind == T_IMPORT && kind == T_STRING) {
                QString singleImportText = searchImportText.mid(tk.start(), tk.count());
                manipulator.replace(tk.start(), tk.count(), QStringLiteral("(\n\t\"") + m_importText + QStringLiteral("\"\n\t") + singleImportText + QStringLiteral("\n)\n"));
                return;
            } else if (prevKind == T_IMPORT) {
                manipulator.replace(prevKeywordEnd, 0, QStringLiteral(" (\n\t\"") + m_importText + QStringLiteral("\"\n)\n"));
                return;
            }
            prevKind = kind;
            prevKeywordEnd = tk.end();
        }

        // No import keyword found
        if (packageKeywordEnd == -1) {
            manipulator.replace(0, 0, QStringLiteral("import (\n\t\"") + m_importText + "\"\n)\n");
        } else {
            QChar ch = searchImportText[++packageKeywordEnd];
            while (packageKeywordEnd < searchImportText.length() && ch != QLatin1Char('\n'))
                ch = searchImportText[++packageKeywordEnd];
            manipulator.replace(packageKeywordEnd, 0, QStringLiteral("\n\nimport (\n\t\"") + m_importText + "\"\n)\n");
        }
    }

private:
    QString m_importText;
};

class GoAssistProposalModel : public TextEditor::GenericProposalModel
{
public:
    GoAssistProposalModel(const QList<TextEditor::AssistProposalItemInterface *> &items)
        : TextEditor::GenericProposalModel()
    { loadContent(items); }

    bool keepPerfectMatch(TextEditor::AssistReason reason) const override
    { return reason != TextEditor::IdleEditor; }
};

GoCompletionAssistProvider::GoCompletionAssistProvider(QObject *parent)
    : TextEditor::CompletionAssistProvider(parent)
{ }

TextEditor::IAssistProcessor *GoCompletionAssistProvider::createProcessor() const
{ return new GoCompletionAssistProcessor; }

int GoCompletionAssistProvider::activationCharSequenceLength() const
{ return 1; }

bool GoCompletionAssistProvider::isActivationCharSequence(const QString &sequence) const
{
    QChar ch = sequence.at(0);
    switch (ch.toLatin1()) {
        case '.':
        case ',':
        case '(':
            return true;
    }

    return false;
}

GoCompletionAssistProcessor::GoCompletionAssistProcessor()
    : TextEditor::IAssistProcessor()
    , m_snippetCollector(Go::Constants::GO_SNIPPETS_GROUP_ID,
                         QIcon(QLatin1String(":/texteditor/images/snippet.png")))
{ }

GoCompletionAssistProcessor::~GoCompletionAssistProcessor()
{ }

TextEditor::IAssistProposal *GoCompletionAssistProcessor::perform(const TextEditor::AssistInterface *interface_)
{
    m_interface.reset(static_cast<const GoCompletionAssistInterface *>(interface_));

    if (m_interface->reason() != TextEditor::ExplicitlyInvoked) {
        QTextCursor cursor(m_interface->textDocument());
        cursor.setPosition(m_interface->position());
        if (GoLexer::tokenUnderCursorIsLiteralOrComment(cursor))
            return 0;
    }

    if (!m_interface->source().isNull()) {
        QChar triggerChar = m_interface->characterAt(m_interface->position() - 1);
        if (triggerChar == QLatin1Char('(') || triggerChar == QLatin1Char(',')) {
            GoTools::GoFunctionHintAssistVisitor functionHint(m_interface->actualSource());
            QStringList functionArgs = functionHint.functionArguments(m_interface->position() - 1);
            if (functionArgs.isEmpty())
                return 0;
            TextEditor::FunctionHintProposalModelPtr model(new GoFunctionHintModel(functionArgs));
            TextEditor::IAssistProposal *proposal = new TextEditor::FunctionHintProposal(functionHint.lparenPosition() + 1, model);
            return proposal;
        }
        bool isDotTrigger = triggerChar == QLatin1Char('.');
        if (!isDotTrigger && !(triggerChar.isLetterOrNumber() || triggerChar == QLatin1Char('_') || triggerChar == QLatin1Char('$')))
            return 0;
        bool isGlobalCompletion = !isDotTrigger;
        int startOfName = m_interface->position();
        int pos = startOfName;
        if (!isDotTrigger) {
            QChar tst = m_interface->characterAt(--startOfName);
            while (m_interface->characterAt(startOfName).isLetterOrNumber() || tst == QLatin1Char('_') || tst == QLatin1Char('$'))
                tst = m_interface->characterAt(startOfName--);
            pos = startOfName;
            startOfName++;
            while (pos > 0) {
                tst = m_interface->characterAt(pos--);
                if (tst.isSpace())
                    continue;
                if (tst == QLatin1Char('.')) {
                    isGlobalCompletion = false;
                    while (pos > 0) {
                        tst = m_interface->characterAt(pos);
                        if (tst.isSpace()) {
                            pos--;
                            continue;
                        }
                        break;
                    }
                }
                break;
            }
        } else {
            pos -= 2;
            while (pos > 0) {
                QChar tst = m_interface->characterAt(pos);
                if (tst.isSpace()) {
                    pos--;
                    continue;
                }
                break;
            }
        }
        GoTools::GoCompletionAssistVisitor visitor(isGlobalCompletion
                                                   ? m_interface->source()
                                                   : m_interface->actualSource(),
                                                   m_completions);
        visitor.fillCompletions(isGlobalCompletion, isGlobalCompletion
                                                    ? startOfName
                                                    : pos);
        if (isGlobalCompletion && !visitor.inImportSection()) {
            // add packages with proposed imports
            addImports();
            // add snippets
            m_completions.append(m_snippetCollector.collect());
            // add keywords and other builtins
            Go::GoIconProvider *iconProvider = Go::GoIconProvider::instance();
            for (const QString &builting: GoLexer::builtins()) {
                TextEditor::AssistProposalItem *item = new TextEditor::AssistProposalItem();
                item->setText(builting);
                item->setIcon(iconProvider->icon(Go::GoIconProvider::Keyword));
                m_completions.append(item);
            }

        }
        if (!m_completions.isEmpty()) {
            TextEditor::GenericProposalModelPtr model(new GoAssistProposalModel(m_completions));
            TextEditor::IAssistProposal *proposal = new TextEditor::GenericProposal(startOfName, model);
            return proposal;
        }
    }

    return 0;
}

QString C_UNDERSCORE(QStringLiteral("_"));

void GoCompletionAssistProcessor::addImports()
{
    Go::GoIconProvider *iconProvider = Go::GoIconProvider::instance();

    QSet <QString> importedAliases;
    for (const GoTools::GoSource::Import &import: m_interface->source()->imports()) {
        if (import.alias != C_UNDERSCORE) {
            TextEditor::AssistProposalItem *item = new TextEditor::AssistProposalItem;;
            item->setText(import.alias);
            item->setIcon(iconProvider->icon(Go::GoIconProvider::Package));
            m_completions.append(item);
            importedAliases.insert(import.alias);
        }
    }

    for (const GoTools::GoPackageKey &pk: GoTools::GoCodeModelManager::instance()->indexedPackageDirs()) {
        if (!importedAliases.contains(pk.second)) {
            QString candidatePkg;
            if (pk.first.isEmpty())
                candidatePkg = pk.second;
            else
                candidatePkg = pk.first + QLatin1Char('/') + pk.second;
            GoWithImportAssistProposalItem *item = new GoWithImportAssistProposalItem(pk.second,
                                                                                      QStringLiteral("Package: ") + candidatePkg,
                                                                                      iconProvider->icon(Go::GoIconProvider::PackageImport),
                                                                                      candidatePkg);
            m_completions.append(item);
        }
    }
}

GoCompletionAssistInterface::GoCompletionAssistInterface(const QString &filePath,
                                                         QTextDocument *textDocument,
                                                         int position,
                                                         TextEditor::AssistReason reason,
                                                         GoDocument *doc)
    : TextEditor::AssistInterface(textDocument, position, filePath, reason)
    , m_doc(doc)
    , m_revision(textDocument->revision())
    , m_source(doc->source())
{ }

GoTools::GoSource::Ptr GoCompletionAssistInterface::source() const
{ return m_source; }

GoTools::GoSource::Ptr GoCompletionAssistInterface::actualSource() const
{ return m_doc->actualSource(m_revision); }

}   // namespace Internal
}   // namespace GoEditor
