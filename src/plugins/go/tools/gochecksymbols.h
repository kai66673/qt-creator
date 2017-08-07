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

#include "gosemantichighlighter.h"
#include "token.h"
#include "scopevisitor.h"

#include <texteditor/semantichighlighter.h>

#include <QFutureInterface>
#include <QObject>
#include <QRunnable>
#include <QThreadPool>

#include <stack>

namespace GoTools {

class GoCheckSymbols
        : public QObject
        , public ScopeSwitchVisitor
        , public QRunnable
        , public QFutureInterface<TextEditor::HighlightingResult>
{
    Q_OBJECT

public:
    typedef TextEditor::HighlightingResult Result;
    typedef GoSemanticHighlighter::Kind Kind;
    typedef QFuture<Result> Future;

    explicit GoCheckSymbols(GoSource *source);

    Future start()
    {
        this->setRunnable(this);
        this->reportStarted();
        Future future = this->future();
        QThreadPool::globalInstance()->start(this, QThread::LowestPriority);
        return future;
    }

    virtual void run();

    Kind kindForSymbol(const Symbol *symbol) const;
    void addUse(const IdentAST *ast, Kind kind);

    const Type *acceptCompositLiteral(const CompositeLitAST *ast);

protected:
    void flush();
    void addUseCheckFirstLine(IdentAST *ast, Kind kind);

    virtual bool preVisit(AST *) override;
    virtual void postVisit(AST *) override { }

    virtual bool visit(ImportSpecAST *ast) override;

    virtual bool visit(FieldAST *ast) override;
    virtual bool visit(TypeIdentAST *ast) override;
    virtual bool visit(PackageTypeAST *ast) override;
    virtual bool visit(SelectorExprAST *ast) override;
    virtual bool visit(CompositeLitAST *ast) override;
    virtual bool visit(KeyValueExprAST *ast) override;
    virtual bool visit(IdentAST *ast) override;
    virtual bool visit(DeclIdentAST *ast) override;

private:
    std::stack<const Type *> m_nestedCimpositLitType;

    GoSource *m_source;
    Control *m_control;
    std::vector<Token> *_tokens;

    QVector<Result> _usages;
    int _chunkSize;
};

}   // namespace GoTools
