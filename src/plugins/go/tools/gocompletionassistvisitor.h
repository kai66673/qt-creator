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

#include <texteditor/codeassist/assistproposaliteminterface.h>

#include <stack>

namespace GoTools {

class GoCompletionAssistVisitor: protected ASTVisitor, public ExprTypeResolver
{
public:
    GoCompletionAssistVisitor(GoSource::Ptr doc, QList<TextEditor::AssistProposalItemInterface *> &completions);

    void fillCompletions(bool isGlobalCompletion, unsigned pos);

    bool inImportSection() const;

protected:
    virtual bool preVisit(AST *) override;

    virtual bool visit(FuncDeclAST *ast) override;
    virtual bool visit(BlockStmtAST *ast) override;
    virtual bool visit(IfStmtAST *ast) override;
    virtual bool visit(RangeStmtAST *ast) override;
    virtual bool visit(ForStmtAST *ast) override;
    virtual bool visit(TypeSwitchStmtAST *ast) override;
    virtual bool visit(SwitchStmtAST *ast) override;
    virtual bool visit(CaseClauseAST *ast) override;
    virtual bool visit(CompositeLitAST *ast) override;

    virtual bool visit(DeclIdentAST *ast) override;
    virtual bool visit(IdentAST *ast) override;
    virtual bool visit(PackageTypeAST *ast) override;
    virtual bool visit(SelectorExprAST *ast) override;

private:
    GoSource::Ptr m_doc;

    std::vector<Token> *_tokens;
    std::stack<const Type *> m_nestedCimpositLitType;

    QList<TextEditor::AssistProposalItemInterface *> &m_completions;
    bool m_ended;
    bool m_inImportSection;
    bool m_isGlobalCompletion;
    unsigned m_pos;
};

}   // namespace GoTools
