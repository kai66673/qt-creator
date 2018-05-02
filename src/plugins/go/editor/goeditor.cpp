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

#include "goeditor.h"
#include "goautocompleter.h"
#include "goeditorconstants.h"
#include "goconstants.h"
#include "godocument.h"
#include "golexer.h"
#include "goplugin.h"
#include "gohoverhandler.h"
#include "gocompletionassist.h"
#include "goeditoroutline.h"
#include "gohighlighter.h"
#include "goindenter.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/fontsettings.h>
#include <utils/fileutils.h>
#include <utils/qtcassert.h>

#include <QMenu>
#include <QDir>

using namespace TextEditor;

namespace GoEditor {
namespace Internal {

/*****************************************************************************
 * GoEditorWidgetPrivate
 *****************************************************************************/

class GoEditorWidgetPrivate
{
public:
    GoEditorWidgetPrivate(GoEditorWidget *q)
        : m_q(q)
        , m_goEditorOutline(new GoTools::GoEditorOutline(q))
        , m_goDocument(qobject_cast<GoDocument *>(q->textDocument()))
    { }

    void updateSemanticFormatsFromFontSettings() {
        const TextEditor::FontSettings &fontSettings = m_q->textDocument()->fontSettings();
        m_warningFormat = fontSettings.toTextCharFormat(TextEditor::C_WARNING);
        m_errorFormat = fontSettings.toTextCharFormat(TextEditor::C_ERROR);
    }

    void showDiagnosticMessages(Core::Id kind, const QList<GoTools::GoSource::DiagnosticMessage> &messages) {
        QList<QTextEdit::ExtraSelection> selections;
        QTextDocument *document = m_q->document();

        foreach (const GoTools::GoSource::DiagnosticMessage &message, messages) {
            const int pos = document->findBlockByNumber(message.line() - 1).position() + message.column() - 1;
            if (pos < 0)
                continue;

            QTextCursor cursor(document);
            cursor.setPosition(pos);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, message.length());

            QTextEdit::ExtraSelection sel;
            sel.cursor = cursor;
            sel.format = message.level() == GoTools::GoSource::DiagnosticMessage::Warning
                         ? m_warningFormat
                         : m_errorFormat;
            sel.format.setToolTip(message.text());
            selections.append(sel);
        }

        m_q->setExtraSelections(kind, selections);
    }

public:
    GoEditorWidget *m_q;
    GoTools::GoEditorOutline *m_goEditorOutline;
    GoDocument *m_goDocument;
    QTextCharFormat m_warningFormat;
    QTextCharFormat m_errorFormat;
};

/*****************************************************************************
 * GoEditor
 *****************************************************************************/

GoEditor::GoEditor()
    : TextEditor::BaseTextEditor()
{
    addContext(Constants::GOEDITOR_ID);
}

void GoEditor::decorateEditor(TextEditorWidget *editor)
{
    editor->textDocument()->setSyntaxHighlighter(new GoHighlighter);
    editor->textDocument()->setIndenter(new GoIndenter);
    editor->setAutoCompleter(new GoAutoCompleter);
}

/*****************************************************************************
 * GoEditorFactory
 *****************************************************************************/

GoEditorFactory::GoEditorFactory()
{
    setId(Constants::GOEDITOR_ID);
    setDisplayName(tr(Constants::GOEDITOR_DISPLAY_NAME));
    addMimeType(QLatin1String(Go::Constants::GO_MIMETYPE));

    setDocumentCreator([]() { return new GoDocument; });
    setEditorWidgetCreator([]() { return new GoEditorWidget; });
    setEditorCreator([]() { return new GoEditor; });
    setAutoCompleterCreator([]() { return new GoAutoCompleter; });

    setCommentDefinition(Utils::CommentDefinition::CppStyle);
    setCodeFoldingSupported(true);
    setMarksVisible(true);
    setParenthesesMatchingEnabled(true);

    setEditorActionHandlers(TextEditorActionHandler::Format
                          | TextEditorActionHandler::UnCommentSelection
                          | TextEditorActionHandler::UnCollapseAll
                          | TextEditorActionHandler::FollowSymbolUnderCursor);

    addHoverHandler(new GoHoverHandler);
}

/*****************************************************************************
 * GoEditorWidget
 *****************************************************************************/

GoEditorWidget::GoEditorWidget()
    : d(new GoEditorWidgetPrivate(this))
{ }

GoEditorWidget::~GoEditorWidget()
{ }

GoTools::GoEditorOutline *GoEditorWidget::outline() const
{ return d->m_goEditorOutline; }

AssistInterface *GoEditorWidget::createAssistInterface(AssistKind kind, AssistReason reason) const
{
    if (kind == Completion) {
        return new GoCompletionAssistInterface(textDocument()->filePath().toString(),
                                               document(),
                                               position(),
                                               reason,
                                               d->m_goDocument);
    } else {
        return TextEditorWidget::createAssistInterface(kind, reason);
    }

    return 0;
}

QString GoEditorWidget::evaluateIdentifierTypeDescription(int pos)
{ return d->m_goDocument->evaluateIdentifierTypeDescription(pos); }

void GoEditorWidget::findUsages()
{ d->m_goDocument->findUsages(textCursor().position()); }

void GoEditorWidget::renameSymbolUnderCursor()
{ d->m_goDocument->renameSymbolUnderCursor(textCursor().position()); }

Utils::Link GoEditorWidget::findLinkAt(const QTextCursor &tc, bool /*resolveTarget*/, bool /*inNextSplit*/)
{ return d->m_goDocument->findLinkAt(tc); }

void GoEditorWidget::finalizeInitialization()
{
    d->updateSemanticFormatsFromFontSettings();

    d->m_goDocument = qobject_cast<GoDocument *>(textDocument());

    // function combo box sorting
    connect(Go::Internal::GoPlugin::instance(), &Go::Internal::GoPlugin::outlineSortingChanged,
            outline(), &GoTools::GoEditorOutline::setSorted);

    connect(this, &QPlainTextEdit::cursorPositionChanged,
            d->m_goEditorOutline, &GoTools::GoEditorOutline::updateIndex);
    connect(d->m_goDocument, &GoDocument::goDocumentUpdated,
            this, &GoEditorWidget::onGoDocumentUpdated);

    insertExtraToolBarWidget(TextEditorWidget::Left, d->m_goEditorOutline->widget());
}

void GoEditorWidget::finalizeInitializationAfterDuplication(TextEditorWidget *other)
{
    QTC_ASSERT(other, return);
    GoEditorWidget *goEditorWidget = qobject_cast<GoEditorWidget *>(other);
    QTC_ASSERT(goEditorWidget, return);

    if (auto doc = d->m_goDocument->source())
        d->m_goEditorOutline->update(doc);
}

void GoEditorWidget::onGoDocumentUpdated(GoTools::GoSource::Ptr doc)
{
    if (doc->revision() != document()->revision())
        return;

    d->m_goEditorOutline->update(doc);
    d->showDiagnosticMessages(TextEditorWidget::CodeWarningsSelection, doc->diagnosticMessages());
}

} // namespace Internal
} // namespace GoEditor
