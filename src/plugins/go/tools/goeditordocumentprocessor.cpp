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
#include "gocodemodelmanager.h"
#include "goeditordocumentprocessor.h"
#include "gochecksymbols.h"
#include "gosettings.h"
#include "linkundercursor.h"
#include "symbolundercursordescriber.h"
//#include "findreferences.h"

#include <texteditor/textdocument.h>
#include <utils/runextensions.h>
#include <utils/fileutils.h>

namespace GoTools {

GoEditorDocumentProcessor::GoEditorDocumentProcessor(TextEditor::TextDocument *document,
                                                     QObject *parent)
    : QObject(parent)
    , m_textDocument(document)
    , m_parser(new GoEditorDocumentParser(document->filePath().toString()))
    , m_semanticHighlighter(new GoSemanticHighlighter(document))
    , m_forceSemanticRehighligting(false)
{
    m_semanticHighlighter->setHighlightingRunner(
                [this]() -> QFuture<TextEditor::HighlightingResult> {
                    GoCheckSymbols *checkSymbols = new GoCheckSymbols(m_source.data());
                    return checkSymbols->start();
                });

    connect(m_parser.data(), &GoEditorDocumentParser::finished,
            this, &GoEditorDocumentProcessor::onParserFinished);
}

GoEditorDocumentProcessor::~GoEditorDocumentProcessor()
{
    m_parserFuture.cancel();
    m_parserFuture.waitForFinished();
}

void GoEditorDocumentProcessor::run(const QByteArray &source, unsigned revision)
{
    GoCodeModelManager *model = GoCodeModelManager::instance();
    const GoLang::GoGeneralSettings &settings = GoLang::GoSettings::generalSettings();
    m_parserFuture = Utils::runAsync(model->sharedThreadPool(),
                                     runParser, m_parser,
                                     source, revision,
                                     settings.goRoot(),
                                     settings.goPath());
}

bool GoEditorDocumentProcessor::isParserRunning() const
{
    return m_parserFuture.isRunning();
}

void GoEditorDocumentProcessor::semanticRehighlight()
{
    m_semanticHighlighter->updateFormatMapFromFontSettings();

    if (m_source.isNull() || m_source->revision() != m_textDocument->document()->revision()
            || !m_semanticHighlighter->revision() /*First (big) package update not finished*/)
        return;

    m_semanticHighlighter->run();
}

void GoEditorDocumentProcessor::editorDocumentTimerRestarted()
{ }

void GoEditorDocumentProcessor::findLinkAt(const QTextCursor &tc, Utils::ProcessLinkCallback &&processLinkCallback)
{
    if (isSourceReady()) {
        LinkUnderCursor linkFinder(m_source);
        return processLinkCallback(linkFinder.link(tc.position()));
    }

    return processLinkCallback(Utils::Link());
}

QString GoEditorDocumentProcessor::evaluateIdentifierTypeDescription(int pos)
{
    if (isSourceReady()) {
        SymbolUnderCursorDescriber describer(m_source);
        return describer.description(pos);
    }

    return QString();
}

void GoEditorDocumentProcessor::findUsages(int pos)
{
    if (isSourceReady())
        GoCodeModelManager::instance()->findReferences(m_source, pos);
}

void GoEditorDocumentProcessor::renameSymbolUnderCursor(int pos)
{
    if (isSourceReady())
        GoCodeModelManager::instance()->renameSymbolUnderCursor(m_source, pos);
}

GoSource::Ptr GoEditorDocumentProcessor::actualSource(int revision)
{
    while (m_source.isNull() || m_source->revision() != revision
           || m_semanticHighlighter->revision() != revision) {
        QThread::msleep(15);
    }
    return m_source;
}

void GoEditorDocumentProcessor::runParser(QFutureInterface<void> &future,
                                          GoEditorDocumentParser::Ptr parser,
                                          const QByteArray &source, unsigned revision,
                                          const QString &goRoot, const QString &goPath)
{
    future.setProgressRange(0, 1);

    if (!future.isCanceled())
        parser->update(source, revision, goRoot, goPath);

    future.setProgressValue(1);
}

void GoEditorDocumentProcessor::onParserFinished(GoTools::GoSource::Ptr doc)
{
    if (doc.isNull() || doc->revision() != m_textDocument->document()->revision())
        return;

    m_source = doc;

    onPackageCacheUpdated();

    emit goDocumentUpdated(doc);
}

void GoEditorDocumentProcessor::onPackageCacheUpdated()
{
    if (m_source.isNull() || m_source->revision() != m_textDocument->document()->revision())
        return;

    if (m_semanticHighlighter->revision() != m_textDocument->document()->revision() || m_forceSemanticRehighligting) {
        m_forceSemanticRehighligting = false;
        m_semanticHighlighter->run();
    }
}

GoTools::GoSource::Ptr GoEditorDocumentProcessor::source() const
{ return m_source; }

void GoEditorDocumentProcessor::setForceSemanticRehighligting()
{ m_forceSemanticRehighligting = true; }

bool GoEditorDocumentProcessor::isSourceReady() const
{
    if (m_source.isNull())
        return false;

    if (m_source->revision() != m_textDocument->document()->revision())
        return false;

    return m_semanticHighlighter->revision() == m_textDocument->document()->revision() && !m_semanticHighlighter->isRunning();
}

}   // namespace GoTools
