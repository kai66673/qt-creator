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
#include "gosemantichighlighter.h"

#include <utils/qtcassert.h>

#include <QTextDocument>

using namespace TextEditor;

namespace GoTools {

GoSemanticHighlighter::GoSemanticHighlighter(TextDocument *baseTextDocument)
    : QObject(baseTextDocument)
    , m_baseTextDocument(baseTextDocument)
    , m_revision(0)
{
    updateFormatMapFromFontSettings();
}

GoSemanticHighlighter::~GoSemanticHighlighter()
{
    if (m_watcher) {
        disconnectWatcher();
        m_watcher->cancel();
        m_watcher->waitForFinished();
    }
}

void GoSemanticHighlighter::setHighlightingRunner(GoSemanticHighlighter::HighlightingRunner highlightingRunner)
{
    m_highlightingRunner = highlightingRunner;
}

static TextStyles mixinStyle(TextStyle main, TextStyle mixin)
{
    TextStyles res;
    res.mainStyle = main;
    res.mixinStyles.initializeElements();
    res.mixinStyles.push_back(mixin);
    return res;
}

void GoSemanticHighlighter::updateFormatMapFromFontSettings()
{
    QTC_ASSERT(m_baseTextDocument, return);

    const FontSettings &fs = m_baseTextDocument->fontSettings();
    auto declFormat = fs.toTextCharFormat(mixinStyle(TextEditor::C_LOCAL, TextEditor::C_DECLARATION));

    m_formatMap[Type] = fs.toTextCharFormat(TextEditor::C_TYPE);
    m_formatMap[Var] = fs.toTextCharFormat(TextEditor::C_LOCAL);
    m_formatMap[Field] = fs.toTextCharFormat(TextEditor::C_FIELD);
    m_formatMap[Arg] = declFormat;
    m_formatMap[Const] = fs.toTextCharFormat(TextEditor::C_ENUMERATION);
    m_formatMap[Label] = fs.toTextCharFormat(TextEditor::C_LABEL);
    m_formatMap[Func] = fs.toTextCharFormat(TextEditor::C_FUNCTION);
    m_formatMap[Package] = fs.toTextCharFormat(TextEditor::C_PREPROCESSOR);
    m_formatMap[TypeDecl] = fs.toTextCharFormat(mixinStyle(TextEditor::C_TYPE, TextEditor::C_DECLARATION));
    m_formatMap[VarDecl] = declFormat;
    m_formatMap[ConstDecl] = fs.toTextCharFormat(mixinStyle(TextEditor::C_ENUMERATION, TextEditor::C_DECLARATION));
    m_formatMap[FuncDecl] = fs.toTextCharFormat(mixinStyle(TextEditor::C_FUNCTION, TextEditor::C_DECLARATION));
}

int GoSemanticHighlighter::revision() const
{ return m_revision; }

void GoSemanticHighlighter::run()
{
    QTC_ASSERT(m_highlightingRunner, return);

    if (m_watcher) {
        disconnectWatcher();
        m_watcher->cancel();
    }
    m_watcher.reset(new QFutureWatcher<HighlightingResult>);
    connectWatcher();

    m_revision = documentRevision();
    m_watcher->setFuture(m_highlightingRunner());
}

bool GoSemanticHighlighter::isRunning() const
{ return m_watcher.isNull() ? false : m_watcher->isRunning(); }

void GoSemanticHighlighter::onHighlighterResultAvailable(int from, int to)
{
    if (documentRevision() != m_revision)
        return; // outdated
    else if (!m_watcher || m_watcher->isCanceled())
        return; // aborted

    SyntaxHighlighter *highlighter = m_baseTextDocument->syntaxHighlighter();
    QTC_ASSERT(highlighter, return);
    SemanticHighlighter::incrementalApplyExtraAdditionalFormats(highlighter, m_watcher->future(), from, to, m_formatMap);
}

void GoSemanticHighlighter::onHighlighterFinished()
{
    QTC_ASSERT(m_watcher, return);
    if (!m_watcher->isCanceled() && documentRevision() == m_revision) {
        SyntaxHighlighter *highlighter = m_baseTextDocument->syntaxHighlighter();
        if (QTC_GUARD(highlighter)) {
            SemanticHighlighter::clearExtraAdditionalFormatsUntilEnd(highlighter, m_watcher->future());
        }
    }
    m_watcher.reset();
}

void GoSemanticHighlighter::connectWatcher()
{
    typedef QFutureWatcher<HighlightingResult> Watcher;
    connect(m_watcher.data(), &Watcher::resultsReadyAt,
            this, &GoSemanticHighlighter::onHighlighterResultAvailable);
    connect(m_watcher.data(), &Watcher::finished,
            this, &GoSemanticHighlighter::onHighlighterFinished);
}

void GoSemanticHighlighter::disconnectWatcher()
{
    typedef QFutureWatcher<HighlightingResult> Watcher;
    disconnect(m_watcher.data(), &Watcher::resultsReadyAt,
               this, &GoSemanticHighlighter::onHighlighterResultAvailable);
    disconnect(m_watcher.data(), &Watcher::finished,
               this, &GoSemanticHighlighter::onHighlighterFinished);
}

int GoSemanticHighlighter::documentRevision() const
{ return m_baseTextDocument->document()->revision(); }

}   // namespace GoTools
