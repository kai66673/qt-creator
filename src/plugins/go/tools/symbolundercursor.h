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

#include "astvisitor.h"
#include "exprtyperesolver.h"
#include "gosnapshot.h"

#include <texteditor/texteditor.h>

#include <stack>

namespace GoTools {

class SymbolUnderCursor: protected ASTVisitor, public ExprTypeResolver
{
    enum UseReason { Link, DescribeType };

public:
    SymbolUnderCursor(GoSource::Ptr doc);

    TextEditor::TextEditorWidget::Link link(unsigned pos);
    QString typeDescription(unsigned pos);

protected:
    virtual bool preVisit(AST *);

    virtual bool visit(ImportSpecAST *ast);

    virtual bool visit(DeclIdentAST *ast);
    virtual bool visit(IdentAST *ast);
    virtual bool visit(TypeIdentAST *ast);
    virtual bool visit(PackageTypeAST *ast);
    virtual bool visit(SelectorExprAST *ast);
    virtual bool visit(FuncDeclAST *ast);
    virtual bool visit(BlockStmtAST *ast);
    virtual bool visit(IfStmtAST *ast);
    virtual bool visit(RangeStmtAST *ast);
    virtual bool visit(ForStmtAST *ast);
    virtual bool visit(TypeSwitchStmtAST *ast);
    virtual bool visit(SwitchStmtAST *ast);
    virtual bool visit(CaseClauseAST *ast);
    virtual bool visit(CompositeLitAST *ast);
    virtual bool visit(KeyValueExprAST *ast);

private:
    void defineSymbolUnderCursor(UseReason reason = Link);

    GoSource::Ptr m_doc;
    unsigned m_pos;

    std::stack<const Type *> m_nestedCimpositLitType;

    Symbol *m_symbol;
    const Token *m_token;
    UseReason m_useReason;
    QString m_symbolTypeDescription;
    QString m_packageAlias;
    ImportSpecAST *m_importSpec;
};

}   // namespace GoTools
