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

#include "goeditordocumentparser.h"
#include "gosource.h"
#include "gosemantichighlighter.h"

#include <texteditor/texteditor.h>

#include <QFuture>
#include <QObject>
#include <QTextDocument>

#include <functional>

namespace GoTools {

class GoEditorDocumentProcessor : public QObject
{
    Q_OBJECT
public:
    explicit GoEditorDocumentProcessor(TextEditor::TextDocument *document,
                                       QObject *parent = 0);
    virtual ~GoEditorDocumentProcessor();

    void run(const QByteArray &source, unsigned revision);
    bool isParserRunning() const;
    void semanticRehighlight();

    void editorDocumentTimerRestarted();

    void findLinkAt(const QTextCursor &tc, Utils::ProcessLinkCallback &&processLinkCallback);
    QString evaluateIdentifierTypeDescription(int pos);
    void findUsages(int pos);
    void renameSymbolUnderCursor(int pos);
    GoTools::GoSource::Ptr actualSource(int revision);

    using HeaderErrorDiagnosticWidgetCreator = std::function<QWidget*()>;

protected:
    static void runParser(QFutureInterface<void> &future,
                          GoEditorDocumentParser::Ptr parser,
                          const QByteArray &source, unsigned revision,
                          const QString &goRoot, const QString &goPath);

signals:
    void codeWarningsUpdated(unsigned revision,
                             const QList<QTextEdit::ExtraSelection> selections,
                             const HeaderErrorDiagnosticWidgetCreator &creator,
                             const TextEditor::RefactorMarkers &refactorMarkers);

    void goDocumentUpdated(const GoTools::GoSource::Ptr document);

private:
    void onParserFinished(GoTools::GoSource::Ptr doc);

public:
    void onPackageCacheUpdated();
    GoTools::GoSource::Ptr source() const;

    void setForceSemanticRehighligting();

private:
    bool isSourceReady() const;

    TextEditor::TextDocument *m_textDocument;
    QFuture<void> m_parserFuture;
    GoEditorDocumentParser::Ptr m_parser;
    GoTools::GoSource::Ptr m_source;
    QScopedPointer<GoTools::GoSemanticHighlighter> m_semanticHighlighter;
    bool m_forceSemanticRehighligting;
};

}   // namespace GoTools
