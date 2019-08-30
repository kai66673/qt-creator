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

#include "astfwd.h"
#include "translationunit.h"

namespace GoTools {

class ASTVisitor
{
    ASTVisitor(const ASTVisitor &other);
    void operator =(const ASTVisitor &other);

public:
    ASTVisitor(TranslationUnit *unit);
    virtual ~ASTVisitor();

    void accept(AST *ast);

    template <typename _Tp>
    void accept(List<_Tp> *it)
    {
        for (; it && !_traverseFinished; it = it->next)
            accept(it->value);
    }

    virtual bool preVisit(AST *) { return !_traverseFinished; }
    virtual void postVisit(AST *) {}

    virtual bool visit(FileAST *) { return true; }
    virtual bool visit(CommentAST *) { return true; }
    virtual bool visit(FieldAST *) { return true; }
    virtual bool visit(FieldGroupAST *) { return true; }
    virtual bool visit(ImportSpecAST *) { return true; }
    virtual bool visit(VarSpecWithTypeAST *) { return true; }
    virtual bool visit(VarSpecWithValuesAST *) { return true; }
    virtual bool visit(ConstSpecAST *) { return true; }
    virtual bool visit(TypeSpecAST *) { return true; }
    virtual bool visit(BadDeclAST *) { return true; }
    virtual bool visit(GenDeclAST *) { return true; }
    virtual bool visit(FuncDeclAST *) { return true; }
    virtual bool visit(BadExprAST *) { return true; }
    virtual bool visit(ParenExprAST *) { return true; }
    virtual bool visit(RhsExprListAST *) { return true; }
    virtual bool visit(RangeExpAST *) { return true; }
    virtual bool visit(IdentAST *) { return true; }
    virtual bool visit(DeclIdentAST *) { return true; }
    virtual bool visit(BadTypeAST *) { return true; }
    virtual bool visit(ParenTypeAST *) { return true; }
    virtual bool visit(TypeIdentAST *) { return true; }
    virtual bool visit(PackageTypeAST *) { return true; }
    virtual bool visit(SelectorExprAST *) { return true; }
    virtual bool visit(IndexExprAST *) { return true; }
    virtual bool visit(SliceExprAST *) { return true; }
    virtual bool visit(TypeAssertExprAST *) { return true; }
    virtual bool visit(EllipsisAST *) { return true; }
    virtual bool visit(EllipsisTypeAST *) { return true; }
    virtual bool visit(CallExprAST *) { return true; }
    virtual bool visit(StarTypeAST *) { return true; }
    virtual bool visit(StarExprAST *) { return true; }
    virtual bool visit(NewExprAST *) { return true; }
    virtual bool visit(MakeExprAST *) { return true; }
    virtual bool visit(UnaryExprAST *) { return true; }
    virtual bool visit(ArrowUnaryExprAST *) { return true; }
    virtual bool visit(RefUnaryExprAST *) { return true; }
    virtual bool visit(BinaryExprAST *) { return true; }
    virtual bool visit(BinaryPlusExprAST *) { return true; }
    virtual bool visit(KeyValueExprAST *) { return true; }
    virtual bool visit(ArrayTypeAST *) { return true; }
    virtual bool visit(StructTypeAST *) { return true; }
    virtual bool visit(ChanTypeAST *) { return true; }
    virtual bool visit(FuncTypeAST *) { return true; }
    virtual bool visit(InterfaceTypeAST *) { return true; }
    virtual bool visit(MapTypeAST *) { return true; }
    virtual bool visit(BasicLitAST *) { return true; }
    virtual bool visit(StringLitAST *) { return true; }
    virtual bool visit(FuncLitAST *) { return true; }
    virtual bool visit(CompositeLitAST *) { return true; }
    virtual bool visit(EmptyStmtAST *) { return true; }
    virtual bool visit(BadStmtAST *) { return true; }
    virtual bool visit(DeclStmtAST *) { return true; }
    virtual bool visit(LabeledStmtAST *) { return true; }
    virtual bool visit(ExprStmtAST *) { return true; }
    virtual bool visit(SendStmtAST *) { return true; }
    virtual bool visit(IncDecStmtAST *) { return true; }
    virtual bool visit(BlockStmtAST *) { return true; }
    virtual bool visit(AssignStmtAST *) { return true; }
    virtual bool visit(DefineStmtAST *) { return true; }
    virtual bool visit(GoStmtAST *) { return true; }
    virtual bool visit(DeferStmtAST *) { return true; }
    virtual bool visit(ReturnStmtAST *) { return true; }
    virtual bool visit(BranchStmtAST *) { return true; }
    virtual bool visit(IfStmtAST *) { return true; }
    virtual bool visit(CaseClauseAST *) { return true; }
    virtual bool visit(SwitchStmtAST *) { return true; }
    virtual bool visit(TypeSwitchStmtAST *) { return true; }
    virtual bool visit(CommClauseAST *) { return true; }
    virtual bool visit(SelectStmtAST *) { return true; }
    virtual bool visit(ForStmtAST *) { return true; }
    virtual bool visit(RangeStmtAST *) { return true; }

    virtual void endVisit(FileAST *) {}
    virtual void endVisit(CommentAST *) {}
    virtual void endVisit(FieldAST *) {}
    virtual void endVisit(FieldGroupAST *) {}
    virtual void endVisit(ImportSpecAST *) {}
    virtual void endVisit(VarSpecWithTypeAST *) {}
    virtual void endVisit(VarSpecWithValuesAST *) {}
    virtual void endVisit(ConstSpecAST *) {}
    virtual void endVisit(TypeSpecAST *) {}
    virtual void endVisit(BadDeclAST *) {}
    virtual void endVisit(GenDeclAST *) {}
    virtual void endVisit(FuncDeclAST *) {}
    virtual void endVisit(BadExprAST *) {}
    virtual void endVisit(ParenExprAST *) {}
    virtual void endVisit(RhsExprListAST *) {}
    virtual void endVisit(RangeExpAST *) {}
    virtual void endVisit(IdentAST *) {}
    virtual void endVisit(DeclIdentAST *) {}
    virtual void endVisit(BadTypeAST *) {}
    virtual void endVisit(ParenTypeAST *) {}
    virtual void endVisit(TypeIdentAST *) {}
    virtual void endVisit(PackageTypeAST *) {}
    virtual void endVisit(SelectorExprAST *) {}
    virtual void endVisit(IndexExprAST *) {}
    virtual void endVisit(SliceExprAST *) {}
    virtual void endVisit(TypeAssertExprAST *) {}
    virtual void endVisit(EllipsisAST *) {}
    virtual void endVisit(EllipsisTypeAST *) {}
    virtual void endVisit(CallExprAST *) {}
    virtual void endVisit(StarTypeAST *) {}
    virtual void endVisit(StarExprAST *) {}
    virtual void endVisit(NewExprAST *) {}
    virtual void endVisit(MakeExprAST *) {}
    virtual void endVisit(UnaryExprAST *) {}
    virtual void endVisit(ArrowUnaryExprAST *) {}
    virtual void endVisit(RefUnaryExprAST *) {}
    virtual void endVisit(BinaryExprAST *) {}
    virtual void endVisit(BinaryPlusExprAST *) {}
    virtual void endVisit(KeyValueExprAST *) {}
    virtual void endVisit(ArrayTypeAST *) {}
    virtual void endVisit(StructTypeAST *) {}
    virtual void endVisit(ChanTypeAST *) {}
    virtual void endVisit(FuncTypeAST *) {}
    virtual void endVisit(InterfaceTypeAST *) {}
    virtual void endVisit(MapTypeAST *) {}
    virtual void endVisit(BasicLitAST *) {}
    virtual void endVisit(StringLitAST *) {}
    virtual void endVisit(FuncLitAST *) {}
    virtual void endVisit(CompositeLitAST *) {}
    virtual void endVisit(EmptyStmtAST *) {}
    virtual void endVisit(BadStmtAST *) {}
    virtual void endVisit(DeclStmtAST *) {}
    virtual void endVisit(LabeledStmtAST *) {}
    virtual void endVisit(ExprStmtAST *) {}
    virtual void endVisit(SendStmtAST *) {}
    virtual void endVisit(IncDecStmtAST *) {}
    virtual void endVisit(BlockStmtAST *) {}
    virtual void endVisit(AssignStmtAST *) {}
    virtual void endVisit(DefineStmtAST *) {}
    virtual void endVisit(GoStmtAST *) {}
    virtual void endVisit(DeferStmtAST *) {}
    virtual void endVisit(ReturnStmtAST *) {}
    virtual void endVisit(BranchStmtAST *) {}
    virtual void endVisit(IfStmtAST *) {}
    virtual void endVisit(CaseClauseAST *) {}
    virtual void endVisit(SwitchStmtAST *) {}
    virtual void endVisit(TypeSwitchStmtAST *) {}
    virtual void endVisit(CommClauseAST *) {}
    virtual void endVisit(SelectStmtAST *) {}
    virtual void endVisit(ForStmtAST *) {}
    virtual void endVisit(RangeStmtAST *) {}

    TranslationUnit *translationUnit() const { return _translationUnit; }

    bool traverseFinished() const { return _traverseFinished; }
    void setTraverseFinished(bool traverseFinished) { _traverseFinished = traverseFinished; }

protected:
    TranslationUnit *_translationUnit;
    std::vector<Token> *_tokens;
    bool _traverseFinished;
};

}   // namespace GoTools
