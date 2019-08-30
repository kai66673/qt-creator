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

#include <texteditor/semantichighlighter.h>
#include <texteditor/textdocument.h>

#include <QFutureWatcher>
#include <QObject>
#include <QTextCharFormat>

#include <functional>

namespace GoTools {

class GoSemanticHighlighter : public QObject
{
    Q_OBJECT

public:
    enum Kind {
        Unknown = 0,
        Type,
        Var,
        Field,
        Arg,
        Const,
        Label,
        Func,
        Package,
        TypeDecl,
        VarDecl,
        ConstDecl,
        FuncDecl
    };
    typedef std::function<QFuture<TextEditor::HighlightingResult> ()> HighlightingRunner;

public:
    explicit GoSemanticHighlighter(TextEditor::TextDocument *baseTextDocument);
    ~GoSemanticHighlighter();

    void setHighlightingRunner(HighlightingRunner highlightingRunner);
    void updateFormatMapFromFontSettings();

    int revision() const;
    void run();

    bool isRunning() const;

private:
    void onHighlighterResultAvailable(int from, int to);
    void onHighlighterFinished();

    void connectWatcher();
    void disconnectWatcher();

    int documentRevision() const;

private:
    TextEditor::TextDocument *m_baseTextDocument;

    int m_revision;
    QScopedPointer<QFutureWatcher<TextEditor::HighlightingResult>> m_watcher;
    QHash<int, QTextCharFormat> m_formatMap;

    HighlightingRunner m_highlightingRunner;
};

}   // namespace GoTools
