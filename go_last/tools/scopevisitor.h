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
#include "resolvecontext.h"

namespace GoTools {

class ScopeSwitchVisitor: public ASTVisitor , public ResolveContext
{
public:
    ScopeSwitchVisitor(GoSource *source, bool protectCache = true);

protected:
    virtual bool visit(FuncDeclAST *ast) override;
    virtual bool visit(BlockStmtAST *ast) override;
    virtual bool visit(IfStmtAST *ast) override;
    virtual bool visit(RangeStmtAST *ast) override;
    virtual bool visit(ForStmtAST *ast) override;
    virtual bool visit(TypeSwitchStmtAST *ast) override;
    virtual bool visit(SwitchStmtAST *ast) override;
    virtual bool visit(CaseClauseAST *ast) override;
};

class ScopePositionVisitor: protected ASTVisitor, public ResolveContext
{
public:
    ScopePositionVisitor(GoSource *source, bool protectCache = true);

    template <typename _Tp>
    void acceptForPosition(List<_Tp> *it)
    {
        for (; it; it = it->next) {
            switch (it->value->positionRelation(m_pos, _tokens)) {
                case Contain:
                    accept(it->value);
                    _traverseFinished = true;
                    return;
                case Before:
                    _traverseFinished = true;
                    return;
                case After:
                    break;
            }
        }
    }

protected:
    virtual bool visit(FuncDeclAST *ast) override;
    virtual bool visit(BlockStmtAST *ast) override;
    virtual bool visit(IfStmtAST *ast) override;
    virtual bool visit(RangeStmtAST *ast) override;
    virtual bool visit(ForStmtAST *ast) override;
    virtual bool visit(TypeSwitchStmtAST *ast) override;
    virtual bool visit(SwitchStmtAST *ast) override;
    virtual bool visit(CaseClauseAST *ast) override;

protected:
    unsigned m_pos;
//    bool m_ended;
};

}   // namespace GoTools
