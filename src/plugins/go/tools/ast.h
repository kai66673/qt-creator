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

#include "memorypool.h"
#include "astfwd.h"
#include "token.h"
#include "types.h"
#include "scope.h"

namespace GoTools {

template <typename _Tp>
class List: public Managed
{
    List(const List &other);
    void operator =(const List &other);

public:
    List()
        : value(_Tp()), next(0)
    { }

    List(const _Tp &value)
        : value(value), next(0)
    { }

    unsigned firstToken() const
    {
        if (value)
            return value->firstToken();

        // ### assert(0);
        return 0;
    }

    unsigned lastToken() const
    {
        _Tp lv = lastValue();

        if (lv)
            return lv->lastToken();

        // ### assert(0);
        return 0;
    }

    _Tp lastValue() const
    {
        _Tp lastValue = 0;

        for (const List *it = this; it; it = it->next) {
            if (it->value)
                lastValue = it->value;
        }

        return lastValue;
    }

    _Tp at(unsigned index) const
    {
        int i = 0;
        const List *it = this;

        for (; it && i < index; it = it->next, i++)
            ;

        return it ? it->value : 0;
    }

    _Tp value;
    List *next;
};

class AST: public Managed
{
    AST(const AST &other);
    void operator =(const AST &other);

public:
    AST();
    virtual ~AST();

    void accept(ASTVisitor *visitor);

    static void accept(AST *ast, ASTVisitor *visitor)
    { if (ast) ast->accept(visitor); }

    template <typename _Tp>
    static void accept(List<_Tp> *it, ASTVisitor *visitor)
    {
        for (; it; it = it->next)
            accept(it->value, visitor);
    }

    virtual FileAST *asFile() { return 0; }
    virtual CommentAST *asComment() { return 0; }
    virtual SpecAST *asSpec() { return 0; }
    virtual FieldAST *asField() { return 0; }
    virtual FieldGroupAST *asFieldGroup() { return 0; }
    virtual ImportSpecAST *asImportSpec() { return 0; }
    virtual VarSpecWithTypeAST *asValueSpecWithType() { return 0; }
    virtual VarSpecWithValuesAST *asVarSpecWithValues() { return 0; }
    virtual ConstSpecAST *asConstSpec() { return 0; }
    virtual TypeSpecAST *asTypeSpec() { return 0; }
    virtual DeclAST *asDecl() { return 0; }
    virtual BadDeclAST *asBadDecl() { return 0; }
    virtual GenDeclAST *asGenDecl() { return 0; }
    virtual FuncDeclAST *asFuncDecl() { return 0; }
    virtual ExprAST *asExpr() { return 0; }
    virtual BadExprAST *asBadExpr() { return 0; }
    virtual ParenExprAST *asParenExpr() { return 0; }
    virtual RhsExprListAST *asRhsExprList() { return 0; }
    virtual RangeExpAST *asRangeExp() { return 0; }
    virtual IdentAST *asIdent() { return 0; }
    virtual TypeIdentAST *asTypeIdent() { return 0; }
    virtual PackageTypeAST *asPackageType() { return 0; }
    virtual SelectorExprAST *asSelectorExpr() { return 0; }
    virtual IndexExprAST *asIndexExpr() { return 0; }
    virtual SliceExprAST *asSliceExpr() { return 0; }
    virtual TypeAssertExprAST *asTypeAssertExpr() { return 0; }
    virtual EllipsisAST *asEllipsis() { return 0; }
    virtual EllipsisTypeAST *asEllipsisType() { return 0; }
    virtual CallExprAST *asCallExpr() { return 0; }
    virtual StarTypeAST *asStarType() { return 0; }
    virtual StarExprAST *asStarExpr() { return 0; }
    virtual UnaryExprAST *asUnaryExpr() { return 0; }
    virtual ArrowUnaryExprAST *asArrowUnaryExpr() { return 0; }
    virtual RefUnaryExprAST *asRefUnaryExpr() { return 0; }
    virtual BinaryExprAST *asBinaryExpr() { return 0; }
    virtual KeyValueExprAST *asKeyValueExpr() { return 0; }
    virtual TypeAST *asType() { return 0; }
    virtual BadTypeAST *asBadType() { return 0; }
    virtual ParenTypeAST *asParenType() { return 0; }
    virtual ArrayTypeAST *asArrayType() { return 0; }
    virtual StructTypeAST *asStructType() { return 0; }
    virtual ChanTypeAST *asChanType() { return 0; }
    virtual FuncTypeAST *asFuncType() { return 0; }
    virtual InterfaceTypeAST *asInterfaceType() { return 0; }
    virtual MapTypeAST *asMapType() { return 0; }
    virtual LitAST *asLit() { return 0; }
    virtual BasicLitAST *asBasicLit() { return 0; }
    virtual StringLitAST *asStringLit() { return 0; }
    virtual FuncLitAST *asFuncLit() { return 0; }
    virtual CompositeLitAST *asCompositeLit() { return 0; }
    virtual StmtAST *asStmt() { return 0; }
    virtual EmptyStmtAST *asEmptyStmt() { return 0; }
    virtual BadStmtAST *asBadStmt() { return 0; }
    virtual DeclStmtAST *asDeclStmt() { return 0; }
    virtual LabeledStmtAST *asLabeledStmt() { return 0; }
    virtual ExprStmtAST *asExprStmt() { return 0; }
    virtual SendStmtAST *asSendStmt() { return 0; }
    virtual IncDecStmtAST *asIncDecStmt() { return 0; }
    virtual BlockStmtAST *asBlockStmt() { return 0; }
    virtual AssignStmtAST *asAssignStmt() { return 0; }
    virtual DefineStmtAST *asDefineStmt() { return 0; }
    virtual GoStmtAST *asGoStmt() { return 0; }
    virtual DeferStmtAST *asDeferStmt() { return 0; }
    virtual ReturnStmtAST *asReturnStmt() { return 0; }
    virtual BranchStmtAST *asBranchStmt() { return 0; }
    virtual IfStmtAST *asIfStmt() { return 0; }
    virtual CaseClauseAST *asCaseClause() { return 0; }
    virtual SwitchStmtAST *asSwitchStmt() { return 0; }
    virtual TypeSwitchStmtAST *asTypeSwitchStmt() { return 0; }
    virtual CommClauseAST *asCommClause() { return 0; }
    virtual SelectStmtAST *asSelectStmt() { return 0; }
    virtual ForStmtAST *asForStmt() { return 0; }
    virtual RangeStmtAST *asRangeStmt() { return 0; }

    virtual unsigned firstToken() const = 0;
    virtual unsigned lastToken() const = 0;

protected:
    virtual void accept0(ASTVisitor *visitor) = 0;
};

class FileAST: public AST
{
public:
    FileScope *scope = 0;

public:
    CommentGroupAST *doc;
    unsigned t_package;
    IdentAST *packageName;
    DeclListAST *importDecls;
    DeclListAST *decls;

public:
    FileAST(CommentGroupAST *doc_ = 0, unsigned t_package_ = 0, IdentAST *packageName_ = 0,
            DeclListAST *importDecls_ = 0, DeclListAST *decls_ = 0)
        : doc(doc_), t_package(t_package_), packageName(packageName_)
        , importDecls(importDecls_), decls(decls_)
    { }

    virtual FileAST *asFile() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class CommentAST: public AST
{
public:
    unsigned t_comment;

public:
    CommentAST(unsigned t_comment_ = 0)
        : t_comment(t_comment_)
    { }

    virtual CommentAST *asComment() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class SpecAST: public AST
{
public:
    SpecAST() {}
    virtual SpecAST *asSpec() { return this; }
};

class FieldAST: public AST
{
public:
    CommentGroupAST *doc;
    DeclIdentListAST *names;
    TypeAST *type;
    StringLitAST *tag;
    CommentGroupAST *comment;

public:
    FieldAST(CommentGroupAST *doc_ = 0, DeclIdentListAST *names_ = 0, TypeAST *type_ = 0, StringLitAST *tag_ = 0, CommentGroupAST *comment_ = 0)
        : doc(doc_), names(names_), type(type_), tag(tag_), comment(comment_)
    { }

    virtual FieldAST *asField() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    QString describe() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class FieldGroupAST: public AST
{
public:
    unsigned t_lparen;
    FieldListAST *fields;
    unsigned t_rparen;

public:
    FieldGroupAST(unsigned t_lparen_ = 0, FieldListAST *fields_ = 0, unsigned t_rparen_ = 0)
        : t_lparen(t_lparen_), fields(fields_), t_rparen(t_rparen_)
    { }

    virtual FieldGroupAST *asFieldGroup() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    QString describe() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class ImportSpecAST: public SpecAST
{
public:
    CommentGroupAST *doc;
    IdentAST *name;
    unsigned t_path;
    CommentGroupAST *comment;

public:
    ImportSpecAST(CommentGroupAST *doc_ = 0, IdentAST *name_ = 0,
                  unsigned t_path_ = 0, CommentGroupAST *comment_ = 0)
        : doc(doc_), name(name_), t_path(t_path_), comment(comment_)
    { }

    virtual ImportSpecAST *asImportSpec() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class VarSpecWithTypeAST: public SpecAST
{
public:
    CommentGroupAST *doc;
    DeclIdentListAST *names;
    TypeAST *type;
    CommentGroupAST *comment;

public:
    VarSpecWithTypeAST(CommentGroupAST *doc_ = 0, DeclIdentListAST *names_ = 0,
                       TypeAST *type_ = 0, CommentGroupAST *comment_ = 0)
        : doc(doc_), names(names_), type(type_), comment(comment_)
    { }

    virtual VarSpecWithTypeAST *asValueSpecWithType() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class VarSpecWithValuesAST: public SpecAST
{
public:
    CommentGroupAST *doc;
    DeclIdentListAST *names;
    ExprListAST *values;
    CommentGroupAST *comment;

public:
    VarSpecWithValuesAST(CommentGroupAST *doc_ = 0, DeclIdentListAST *names_ = 0, ExprListAST *values_ = 0, CommentGroupAST *comment_ = 0)
        : doc(doc_), names(names_), values(values_), comment(comment_)
    { }

public:
    virtual VarSpecWithValuesAST *asVarSpecWithValues() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class ConstSpecAST: public SpecAST
{
public:
    CommentGroupAST *doc;
    DeclIdentListAST *names;
    TypeAST *type;
    ExprListAST *values;
    CommentGroupAST *comment;

public:
    ConstSpecAST(CommentGroupAST *doc_ = 0, DeclIdentListAST *names_ = 0, TypeAST *type_ = 0,
                 ExprListAST *values_ = 0, CommentGroupAST *comment_ = 0)
        : doc(doc_), names(names_), type(type_), values(values_), comment(comment_)
    { }

    virtual ConstSpecAST *asConstSpec() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class TypeSpecAST: public SpecAST, public Type
{
public:
    FileScope *scope = 0;

public:
    CommentGroupAST *doc;
    DeclIdentAST *name;
    TypeAST *type;
    CommentGroupAST *comment;

public:
    TypeSpecAST(CommentGroupAST *doc_ = 0, DeclIdentAST *name_ = 0, TypeAST *type_ = 0, CommentGroupAST *comment_ = 0)
        : doc(doc_), name(name_), type(type_), comment(comment_)
    { }

    virtual TypeSpecAST *asTypeSpec() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    // LookupContext implemntation
    virtual Symbol *lookupMember(const IdentAST *ident, ExprTypeResolver *resolver) const override;
    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ExprTypeResolver *resolver, Predicate) const override;


    // Type implementation
    virtual const Type *indexType(ExprTypeResolver *resolver) const override;
    virtual const Type *elementsType(ExprTypeResolver *resolver) const override;
    virtual const Type *calleeType(int index, ExprTypeResolver *resolver) const override;
    virtual void fillTuple(TupleType *tuple, ExprTypeResolver *resolver) const override;
    virtual const Type *chanValueType() const override;

    virtual QString describe() const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class DeclAST: public AST
{
public:
    DeclAST() {}
    virtual DeclAST *asDecl() { return this; }
};

class BadDeclAST: public DeclAST
{
public:
    unsigned t_first;
    unsigned t_last;

public:
    BadDeclAST(unsigned t_first_, unsigned t_last_)
        : t_first(t_first_), t_last(t_last_)
    { }

    virtual BadDeclAST *asBadDecl() { return this; }

    virtual unsigned firstToken() const { return t_first; }
    virtual unsigned lastToken() const { return t_last; }

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class GenDeclAST: public DeclAST
{
public:
    TokenKind kind;     // IMPORT|CONST|TYPE|VAR

public:
    CommentGroupAST *doc;
    unsigned t_token;   // token index of IMPORT|CONST|TYPE|VAR
    unsigned t_lparen;
    SpecListAST *specs;
    unsigned t_rparen;

public:
    GenDeclAST(CommentGroupAST *doc_ = 0, unsigned t_token_ = 0,
               unsigned t_lparen_ = 0, SpecListAST *specs_ = 0, unsigned t_rparen_ = 0)
        : doc(doc_), t_token(t_token_), t_lparen(t_lparen_), specs(specs_), t_rparen(t_rparen_)
    { }

    virtual GenDeclAST *asGenDecl() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class FuncDeclAST: public DeclAST
{
public:
    Scope *scope = 0;
    Symbol *symbol = 0;

public:
    CommentGroupAST *doc;
    FieldGroupAST *recv;
    DeclIdentAST *name;
    FuncTypeAST *type;
    BlockStmtAST *body;

public:
    FuncDeclAST(CommentGroupAST *doc_ = 0, FieldGroupAST *recv_ = 0, DeclIdentAST *name_ = 0, FuncTypeAST *type_ = 0, BlockStmtAST *body_ = 0)
        : doc(doc_), recv(recv_), name(name_), type(type_), body(body_)
    { }

    virtual FuncDeclAST *asFuncDecl() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class ExprAST: public AST
{
public:
    ExprAST() {}
    virtual ExprAST *asExpr() { return this; }

    virtual void topLevelResolve(ExprTypeResolver *resolver, TupleType *tuple) const;
    virtual const Type *resolve(ExprTypeResolver *, int &) const { return 0; }
};

class BadExprAST: public ExprAST
{
public:
    unsigned t_first;
    unsigned t_last;

public:
    BadExprAST(unsigned t_first_, unsigned t_last_)
        : t_first(t_first_), t_last(t_last_)
    { }

    virtual BadExprAST *asBadExpr() { return this; }

    virtual unsigned firstToken() const { return t_first; }
    virtual unsigned lastToken() const { return t_last; }

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class ParenExprAST: public ExprAST
{
public:
    unsigned t_lparen;
    ExprAST *x;
    unsigned t_rparen;

public:
    ParenExprAST(unsigned t_lparen_, ExprAST *x_ = 0, unsigned t_rparen_ = 0)
        : t_lparen(t_lparen_), x(x_), t_rparen(t_rparen_)
    { }

    virtual ParenExprAST *asParenExpr() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual const Type *resolve(ExprTypeResolver *resolver, int &derefLevel) const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class RhsExprListAST: public ExprAST
{
public:
    TupleType *resolvedType = 0;

public:
    ExprListAST *list;

public:
    RhsExprListAST(ExprListAST *list_)
        : list(list_)
    { }

    virtual RhsExprListAST *asRhsExprList() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    const Type *type(ExprTypeResolver *resolver, int index);

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class RangeExpAST: public ExprAST
{
public:
    TupleType *resolvedType = 0;

public:
    ExprAST *x;

public:
    RangeExpAST(ExprAST *x_) : x(x_) { }

    virtual RangeExpAST *asRangeExp() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    const Type *type(ExprTypeResolver *resolver, int index);

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class IdentAST: public ExprAST
{
public:
    const Identifier *ident;

public:
    unsigned t_identifier;

public:
    IdentAST(unsigned t_identifier_, const Identifier *ident_)
        : ident(ident_), t_identifier(t_identifier_)
    { }

    virtual IdentAST *asIdent() { return this; }

    bool isLookable() const;
    bool isNewKeyword() const;
    bool isMakeKeyword() const;
    bool isNewOrMakeKeyword() const;

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual const Type *resolve(ExprTypeResolver *resolver, int &) const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class DeclIdentAST: public IdentAST
{
public:
    Symbol *symbol = 0;

public:
    DeclIdentAST(unsigned t_identifier_, const Identifier *ident_ = 0)
        : IdentAST(t_identifier_, ident_)
    { }

    virtual DeclIdentAST *asDeclIdent() { return this; }

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class TypeAST: public ExprAST, public Type
{
public:
    TypeAST() {}
    virtual TypeAST *asType() { return this; }

    virtual bool isValidCompositeLiteralType() const = 0;

    // LookupContext implemntation
    virtual Symbol *lookupMember(const IdentAST *, ExprTypeResolver *) const override { return 0; }
    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &,
                                       ExprTypeResolver *, Predicate = 0) const override { }

    // Type implementation
    virtual const Type *indexType(ExprTypeResolver *) const override { return 0; }
    virtual const Type *elementsType(ExprTypeResolver *) const override { return 0; }
    virtual const Type *calleeType(int, ExprTypeResolver *) const override { return 0; }
    virtual const Type *chanValueType() const override { return 0; }

    virtual Symbol *declaration(ExprTypeResolver *) { return 0; }

    virtual void topLevelResolve(ExprTypeResolver *, TupleType *tuple) const override;
    virtual const Type *resolve(ExprTypeResolver *, int &) const override;
};

class BadTypeAST: public TypeAST
{
public:
    unsigned t_first;
    unsigned t_last;

public:
    BadTypeAST(unsigned t_first_, unsigned t_last_)
        : t_first(t_first_), t_last(t_last_)
    { }

    virtual BadTypeAST *asBadType() { return this; }

    virtual unsigned firstToken() const { return t_first; }
    virtual unsigned lastToken() const { return t_last; }

    virtual bool isValidCompositeLiteralType() const { return true; }

    virtual QString describe() const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class ParenTypeAST: public TypeAST
{
public:
    unsigned t_lparen;
    TypeAST *x;
    unsigned t_rparen;

public:
    ParenTypeAST(unsigned t_lparen_, TypeAST *x_ = 0, unsigned t_rparen_ = 0)
        : t_lparen(t_lparen_), x(x_), t_rparen(t_rparen_)
    { }

    virtual ParenTypeAST *asParenType() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual bool isValidCompositeLiteralType() const { return false; }

    virtual QString describe() const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class TypeIdentAST: public TypeAST
{
public:
    Scope *usingScope = 0;

public:
    IdentAST *ident;

public:
    TypeIdentAST(IdentAST *ident_)
        : ident(ident_)
    { }

    virtual TypeIdentAST *asTypeIdent() { return this; }
    virtual IdentAST *asIdent() { return ident; }

    virtual unsigned firstToken() const { return ident->firstToken(); }
    virtual unsigned lastToken() const { return ident->lastToken(); }

    virtual bool isValidCompositeLiteralType() const { return true; }

    virtual Symbol *lookupMember(const IdentAST *ast, ExprTypeResolver *resolver) const override;
    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ExprTypeResolver *resolver, Predicate = 0) const override;

    virtual const Type *elementsType(ExprTypeResolver *resolver) const override;
    virtual const Type *indexType(ExprTypeResolver *resolver) const override;
    virtual const Type *calleeType(int index, ExprTypeResolver *resolver) const override;
    virtual void fillTuple(TupleType *tuple, ExprTypeResolver *resolver) const override;

    virtual QString describe() const override;

    virtual Symbol *declaration(ExprTypeResolver *resolver) override;
    
protected:
    virtual void accept0(ASTVisitor *visitor);
};

class PackageTypeAST: public TypeAST
{
public:
    FileScope *fileScope = 0;

public:
    IdentAST *packageAlias;
    unsigned t_dot;
    IdentAST *typeName;

public:
    PackageTypeAST(IdentAST *packageAlias_, unsigned t_dot_, IdentAST *typeName_)
        : packageAlias(packageAlias_), t_dot(t_dot_), typeName(typeName_)
    { }

    virtual PackageTypeAST *asPackageType() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual bool isValidCompositeLiteralType() const { return true; }

    virtual Symbol *lookupMember(const IdentAST *ast, ExprTypeResolver *resolver) const override;
    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ExprTypeResolver *resolver, Predicate) const override;

    virtual const Type *elementsType(ExprTypeResolver *resolver) const override;
    virtual const Type *indexType(ExprTypeResolver *resolver) const override;
    virtual QString describe() const override;

    virtual Symbol *declaration(ExprTypeResolver *resolver) override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class SelectorExprAST: public ExprAST
{
public:
    ExprAST *x;
    IdentAST *sel;

public:
    SelectorExprAST(ExprAST *x_, IdentAST *sel_ = 0)
        : x(x_), sel(sel_)
    { }

    virtual SelectorExprAST *asSelectorExpr() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual const Type *resolve(ExprTypeResolver *resolver, int &derefLevel) const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class IndexExprAST: public ExprAST
{
public:
    ExprAST *x;
    unsigned t_lbracket;
    ExprAST *index;
    unsigned t_rbracket;

public:
    IndexExprAST(ExprAST *x_ = 0, unsigned t_lbracket_ = 0, ExprAST *index_ = 0, unsigned t_rbracket_ = 0)
        : x(x_), t_lbracket(t_lbracket_), index(index_), t_rbracket(t_rbracket_)
    { }

    virtual IndexExprAST *asIndexExpr() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual const Type *resolve(ExprTypeResolver *resolver, int &derefLevel) const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class SliceExprAST: public ExprAST
{
public:
    bool slice3;

public:
    ExprAST *x;
    unsigned t_lbracket;
    ExprAST *low;
    ExprAST *high;
    ExprAST *max;
    unsigned t_rbracket;

public:
    SliceExprAST(ExprAST *x_ = 0, unsigned t_lbracket_ = 0, ExprAST *low_ = 0, ExprAST *high_ = 0, ExprAST *max_ = 0, unsigned t_rbracket_ = 0)
        : x(x_), t_lbracket(t_lbracket_), low(low_), high(high_), max(max_), t_rbracket(t_rbracket_)
    { }

    virtual SliceExprAST *asSliceExpr() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class TypeAssertExprAST: public ExprAST
{
public:
    ExprAST *x;
    unsigned t_lparen;
    TypeAST *typ;
    unsigned t_rparen;

public:
    TypeAssertExprAST(ExprAST *x_ = 0, unsigned t_lparen_ = 0, TypeAST *typ_ = 0, unsigned t_rparen_ = 0)
        : x(x_), t_lparen(t_lparen_), typ(typ_), t_rparen(t_rparen_)
    { }

    virtual TypeAssertExprAST *asTypeAssertExpr() { return this; }
    virtual TypeAST *asType() { return typ ? typ->asType() : 0; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual Type *resolve(ExprTypeResolver *, int &derefLevel) const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class EllipsisAST: public ExprAST
{
public:
    unsigned t_ellipsis;

public:
    EllipsisAST(unsigned t_ellipsis_)
        : t_ellipsis(t_ellipsis_)
    { }

    virtual EllipsisAST *asEllipsis() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class CallExprAST: public ExprAST
{
public:
    ExprAST *fun;
    unsigned t_lparen;
    ExprListAST *args;
    unsigned t_ellipsis;
    unsigned t_rparen;

public:
    CallExprAST(ExprAST *fun_ = 0, unsigned t_lparen_ = 0, ExprListAST *args_ = 0, unsigned t_ellipsis_ = 0, unsigned t_rparen_ = 0)
        : fun(fun_), t_lparen(t_lparen_), args(args_), t_ellipsis(t_ellipsis_), t_rparen(t_rparen_)
    { }

    virtual CallExprAST *asCallExpr() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual void topLevelResolve(ExprTypeResolver *resolver, TupleType *tuple) const override;
    virtual const Type *resolve(ExprTypeResolver *resolver, int &derefLevel) const override;

protected:
    const Type *tryResolvePeculiarCase(ExprTypeResolver *resolver, int &derefLevel) const;
    virtual void accept0(ASTVisitor *visitor);
};

class StarTypeAST: public TypeAST
{
public:
    unsigned t_star;
    TypeAST *typ;

public:
    StarTypeAST(unsigned t_star_, TypeAST *x_)
        : t_star(t_star_), typ(x_)
    { }

    virtual StarTypeAST *asStarType() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual bool isValidCompositeLiteralType() const { return false; }

    virtual Symbol *lookupMember(const IdentAST *ident, ExprTypeResolver *resolver) const override
    { return typ ? typ->lookupMember(ident, resolver) : 0; }
    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ExprTypeResolver *resolver, Predicate = 0) const override
    { if (typ) typ->fillMemberCompletions(completions, resolver); }
    virtual void fillTuple(TupleType *tuple, ExprTypeResolver *resolver) const override
    { if (typ) typ->fillTuple(tuple, resolver); }

    virtual const Type *baseType() const override { return typ ? typ->baseType() : 0; }
    virtual int refLevel() const override { return typ ? -1 + typ->refLevel() : -1; }

    virtual QString describe() const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class StarExprAST: public ExprAST
{
public:
    unsigned t_star;
    ExprAST *x;

public:
    StarExprAST(unsigned t_star_, ExprAST *x_)
        : t_star(t_star_), x(x_)
    { }

    virtual StarExprAST *asStarExpr() { return this; }
    virtual TypeAST *asType() { return x ? x->asType() : 0; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual bool isValidCompositeLiteralType() const { return false; }

    virtual const Type *resolve(ExprTypeResolver *resolver, int &derefLevel) const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class UnaryExprAST: public ExprAST
{
public:
    unsigned t_op;
    ExprAST *x;

public:
    UnaryExprAST(unsigned t_op_ = 0, ExprAST *x_ = 0)
        : t_op(t_op_), x(x_)
    { }

    virtual UnaryExprAST *asUnaryExpr() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class ArrowUnaryExprAST: public UnaryExprAST
{
public:
    ArrowUnaryExprAST(unsigned t_op_ = 0, ExprAST *x_ = 0)
        : UnaryExprAST(t_op_, x_)
    { }

    virtual ArrowUnaryExprAST *asArrowUnaryExpr() { return this; }

    virtual const Type *resolve(ExprTypeResolver *resolver, int &) const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class RefUnaryExprAST: public UnaryExprAST
{
public:
    RefUnaryExprAST(unsigned t_op_ = 0, ExprAST *x_ = 0)
        : UnaryExprAST(t_op_, x_)
    { }

    virtual RefUnaryExprAST *asRefUnaryExpr() { return this; }

    virtual const Type *resolve(ExprTypeResolver *resolver, int &derefLevel) const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class BinaryExprAST: public ExprAST
{
public:
    ExprAST *x;
    unsigned t_op;
    ExprAST *y;

public:
    BinaryExprAST(ExprAST *x_ = 0, unsigned t_op_ = 0, ExprAST *y_ = 0)
        : x(x_), t_op(t_op_), y(y_)
    { }

    virtual BinaryExprAST *asBinaryExpr() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class KeyValueExprAST: public ExprAST
{
public:
    ExprAST *key;
    unsigned t_colon;
    ExprAST *value;

public:
    KeyValueExprAST(ExprAST *key_ = 0, unsigned t_colon_ = 0, ExprAST *value_ = 0)
        : key(key_), t_colon(t_colon_), value(value_)
    { }

    virtual KeyValueExprAST *asKeyValueExpr() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class EllipsisTypeAST: public TypeAST
{
public:
    unsigned t_ellipsis;
    TypeAST *ellipsisElement;

public:
    EllipsisTypeAST(unsigned t_ellipsis_, TypeAST *ellipsisElement_)
        : t_ellipsis(t_ellipsis_), ellipsisElement(ellipsisElement_)
    { }

    virtual EllipsisTypeAST *asEllipsisType() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual bool isValidCompositeLiteralType() const { return false; }

    virtual const Type *elementsType(ExprTypeResolver *) const override;
    virtual QString describe() const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class ArrayTypeAST: public TypeAST
{
public:
    unsigned t_lbracket;
    ExprAST *len;
    unsigned t_rbracket;
    TypeAST *elementType;

public:
    ArrayTypeAST(unsigned t_lbracket_, ExprAST *len_ = 0, unsigned t_rbracket_ = 0, TypeAST *elementType_ = 0)
        : t_lbracket(t_lbracket_), len(len_), t_rbracket(t_rbracket_), elementType(elementType_)
    { }

    virtual ArrayTypeAST *asArrayType() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual bool isValidCompositeLiteralType() const { return true; }

    virtual const Type *elementsType(ExprTypeResolver *) const override;
    virtual QString describe() const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class ChanTypeAST: public TypeAST
{
public:
    enum ChanDir {
        SEND = 1,
        RECV = 2,
        SEND_OR_RECV = 3
    };
    ChanDir dir;

public:
    unsigned t_chan;
    unsigned t_arrow;
    TypeAST *value;

public:
    ChanTypeAST(unsigned t_chan_, unsigned t_arrow_, TypeAST *value_)
        : t_chan(t_chan_), t_arrow(t_arrow_), value(value_)
    { }

    virtual ChanTypeAST *asChanType() { return this; }

    virtual unsigned firstToken() const { return t_chan; }
    virtual unsigned lastToken() const { return value->lastToken(); }

    virtual bool isValidCompositeLiteralType() const { return false; }

    virtual Type *chanValueType() const override { return value; }

    virtual QString describe() const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class FuncTypeAST: public TypeAST
{
public:
    unsigned t_func;
    FieldGroupAST *params;
    FieldGroupAST *results;

public:
    FuncTypeAST(unsigned t_func_ = 0, FieldGroupAST *params_ = 0, FieldGroupAST *results_ = 0)
        : t_func(t_func_), params(params_), results(results_)
    { }

    virtual FuncTypeAST *asFuncType() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual bool isValidCompositeLiteralType() const { return false; }

    virtual Type *calleeType(int index, ExprTypeResolver *) const override;
    virtual void fillTuple(TupleType *tuple, ExprTypeResolver *) const override;
    virtual QString describe() const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class StructTypeAST: public TypeAST
{
public:
    unsigned t_struct;
    FieldGroupAST *fields;

public:
    StructTypeAST(unsigned t_struct_ = 0, FieldGroupAST *fields_ = 0)
        : t_struct(t_struct_), fields(fields_)
    { }

    virtual StructTypeAST *asStructType() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual bool isValidCompositeLiteralType() const { return true; }

    virtual Symbol *lookupMember(const IdentAST *ast, ExprTypeResolver *resolver) const override;
    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ExprTypeResolver *resolver, Predicate = 0) const override;
    virtual QString describe() const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class InterfaceTypeAST: public TypeAST
{
public:
    bool incomplete;

public:
    unsigned t_interface;
    FieldGroupAST *methods;

public:
    InterfaceTypeAST(unsigned t_interface_ = 0, FieldGroupAST *methods_ = 0)
        : incomplete(true) , t_interface(t_interface_), methods(methods_)
    { }

    virtual InterfaceTypeAST *asInterfaceType() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual bool isValidCompositeLiteralType() const { return false; }

    virtual Symbol *lookupMember(const IdentAST *ast, ExprTypeResolver *resolver) const override;
    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ExprTypeResolver *resolver, Predicate = 0) const override;
    virtual QString describe() const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class MapTypeAST: public TypeAST
{
public:
    unsigned t_map;
    TypeAST *key;
    TypeAST *value;

public:
    MapTypeAST(unsigned t_map_ = 0, TypeAST *key_ = 0, TypeAST *value_ = 0)
        : t_map(t_map_), key(key_), value(value_)
    { }

    virtual MapTypeAST *asMapType() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual bool isValidCompositeLiteralType() const { return true; }

    virtual const Type *elementsType(ExprTypeResolver *) const override;
    virtual const Type *indexType(ExprTypeResolver *) const override;
    virtual QString describe() const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class LitAST: public ExprAST {
public:
    LitAST() {}
    virtual LitAST *asLit() { return this; }
};

class BasicLitAST: public LitAST
{
public:
    unsigned t_value;

public:
    BasicLitAST(unsigned t_value_)
        : t_value(t_value_)
    { }

    virtual BasicLitAST *asBasicLit() { return this; }

    virtual unsigned firstToken() const { return t_value; }
    virtual unsigned lastToken() const { return t_value; }

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class StringLitAST: public BasicLitAST
{
public:
    StringLitAST(unsigned t_value_)
        : BasicLitAST(t_value_)
    { }

    virtual StringLitAST *asStringLit() { return this; }

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class FuncLitAST: public LitAST
{
public:
    FuncTypeAST *type;
    BlockStmtAST *body;

public:
    FuncLitAST(FuncTypeAST *type_ = 0, BlockStmtAST *body_ = 0)
        : type(type_), body(body_)
    { }

    virtual FuncLitAST *asFuncLit() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual Type *resolve(ExprTypeResolver *, int &) const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class CompositeLitAST: public LitAST
{
public:
    ExprAST *type;
    unsigned t_lbrace;
    ExprListAST *elements;
    unsigned t_rbrace;

public:
    CompositeLitAST(ExprAST *type_ = 0, unsigned t_lbrace_ = 0, ExprListAST *elements_ = 0, unsigned t_rbrace_ = 0)
        : type(type_), t_lbrace(t_lbrace_), elements(elements_), t_rbrace(t_rbrace_)
    { }

    virtual CompositeLitAST *asCompositeLit() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    virtual const Type *resolve(ExprTypeResolver *resolver, int &) const override;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class StmtAST: public AST
{
public:
    StmtAST() {}
    virtual StmtAST *asStmt() { return this; }
};

class EmptyStmtAST: public StmtAST
{
public:
    unsigned t_semicolon;

public:
    EmptyStmtAST(unsigned t_semicolon_)
        : t_semicolon(t_semicolon_)
    { }

    virtual EmptyStmtAST *asEmptyStmt() { return this; }

    virtual unsigned firstToken() const { return t_semicolon; }
    virtual unsigned lastToken() const { return t_semicolon; }

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class BadStmtAST: public StmtAST
{
public:
    unsigned t_first;
    unsigned t_last;

public:
    BadStmtAST(unsigned t_first_, unsigned t_last_)
        : t_first(t_first_), t_last(t_last_)
    { }

    virtual BadStmtAST *asBadStmt() { return this; }

    virtual unsigned firstToken() const { return t_first; }
    virtual unsigned lastToken() const { return t_last; }

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class DeclStmtAST: public StmtAST
{
public:
    DeclAST *decl;

public:
    DeclStmtAST(DeclAST *decl_ = 0)
        : decl(decl_)
    { }

    virtual DeclStmtAST *asDeclStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class LabeledStmtAST: public StmtAST
{
public:
    IdentAST *label;
    unsigned t_colon;
    StmtAST *stmt;

public:
    LabeledStmtAST(IdentAST *label_ = 0, unsigned t_colon_ = 0, StmtAST *stmt_ = 0)
        : label(label_), t_colon(t_colon_), stmt(stmt_)
    { }

    virtual LabeledStmtAST *asLabeledStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class ExprStmtAST: public StmtAST
{
public:
    ExprAST *x;

public:
    ExprStmtAST(ExprAST *x_ = 0)
        : x(x_)
    { }

    virtual ExprStmtAST *asExprStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class SendStmtAST: public StmtAST
{
public:
    ExprAST *chan;
    unsigned t_arrow;
    ExprAST *value;

public:
    SendStmtAST(ExprAST *chan_ = 0, unsigned t_arrow_ = 0, ExprAST *value_ = 0)
        : chan(chan_), t_arrow(t_arrow_), value(value_)
    { }

    virtual SendStmtAST *asSendStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class IncDecStmtAST: public StmtAST
{
public:
    ExprAST *x;
    unsigned t_pos;

public:
    IncDecStmtAST(ExprAST *x_ = 0, unsigned t_pos_ = 0)
        : x(x_), t_pos(t_pos_)
    { }

    virtual IncDecStmtAST *asIncDecStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class BlockStmtAST: public StmtAST
{
public:
    Scope *scope = 0;

public:
    unsigned t_lbrace;
    StmtListAST *list;
    unsigned t_rbrace;

public:
    BlockStmtAST(unsigned t_lbrace_ = 0, StmtListAST *list_ = 0, unsigned t_rbrace_ = 0)
        : t_lbrace(t_lbrace_), list(list_), t_rbrace(t_rbrace_)
    { }

    virtual BlockStmtAST *asBlockStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class AssignStmtAST: public StmtAST
{
public:
    ExprListAST *lhs;
    unsigned t_assign;
    ExprListAST *rhs;

public:
    AssignStmtAST(ExprListAST *lhs_ = 0, unsigned t_assign_ = 0, ExprListAST *rhs_ = 0)
        : lhs(lhs_), t_assign(t_assign_), rhs(rhs_)
    { }

    virtual AssignStmtAST *asAssignStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class DefineStmtAST: public AssignStmtAST
{
public:
    DefineStmtAST(ExprListAST *lhs_ = 0, unsigned t_assign_ = 0, ExprListAST *rhs_ = 0)
        : AssignStmtAST(lhs_, t_assign_, rhs_)
    { }

    virtual DefineStmtAST *asDefineStmt() { return this; }

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class GoStmtAST: public StmtAST
{
public:
    unsigned t_go;
    CallExprAST *call;

public:
    GoStmtAST(unsigned t_go_ = 0, CallExprAST *call_ = 0)
        : t_go(t_go_), call(call_)
    { }

    virtual GoStmtAST *asGoStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class DeferStmtAST: public StmtAST
{
public:
    unsigned t_defer;
    CallExprAST *call;

public:
    DeferStmtAST(unsigned t_defer_ = 0, CallExprAST *call_ = 0)
        : t_defer(t_defer_), call(call_)
    { }

    virtual DeferStmtAST *asDeferStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class ReturnStmtAST: public StmtAST
{
public:
    unsigned t_return;
    ExprListAST *results;

public:
    ReturnStmtAST(unsigned t_return_ = 0, ExprListAST *results_ = 0)
        : t_return(t_return_), results(results_)
    { }

    virtual ReturnStmtAST *asReturnStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class BranchStmtAST: public StmtAST
{
public:
    unsigned t_pos;
    IdentAST *label;

public:
    BranchStmtAST(unsigned t_pos_ = 0, IdentAST *label_ = 0)
        : t_pos(t_pos_), label(label_)
    { }

    virtual BranchStmtAST *asBranchStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class IfStmtAST: public StmtAST
{
public:
    Scope *scope = 0;

public:
    unsigned t_if;
    StmtAST *init;
    ExprAST *cond;
    BlockStmtAST *body;
    StmtAST *elseStmt;

public:
    IfStmtAST(unsigned t_if_ = 0, StmtAST *init_ = 0, ExprAST *cond_ = 0, BlockStmtAST *body_ = 0, StmtAST *elseStmt_ = 0)
        : t_if(t_if_), init(init_), cond(cond_), body(body_), elseStmt(elseStmt_)
    { }

    virtual IfStmtAST *asIfStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class CaseClauseAST: public StmtAST
{
public:
    Scope *scope = 0;

public:
    unsigned t_case;
    ExprListAST *list;
    unsigned t_colon;
    StmtListAST *body;

public:
    CaseClauseAST(unsigned t_case_ = 0, ExprListAST *list_ = 0, unsigned t_colon_ = 0, StmtListAST *body_ = 0)
        : t_case(t_case_), list(list_), t_colon(t_colon_), body(body_)
    { }

    virtual CaseClauseAST *asCaseClause() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class SwitchStmtAST: public StmtAST
{
public:
    Scope *scope = 0;

public:
    unsigned t_switch;
    StmtAST *init;
    ExprAST *tag;
    BlockStmtAST *body;

public:
    SwitchStmtAST(unsigned t_switch_ = 0, StmtAST *init_ = 0, ExprAST *tag_ = 0, BlockStmtAST *body_ = 0)
        : t_switch(t_switch_), init(init_), tag(tag_), body(body_)
    { }

    virtual SwitchStmtAST *asSwitchStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class TypeSwitchStmtAST: public StmtAST
{
public:
    Scope *scope = 0;

public:
    unsigned t_switch;
    StmtAST *init;
    StmtAST *assign;
    BlockStmtAST *body;

public:
    TypeSwitchStmtAST(unsigned t_switch_ = 0, StmtAST *init_ = 0, StmtAST *assign_ = 0, BlockStmtAST *body_ = 0)
        : t_switch(t_switch_), init(init_), assign(assign_), body(body_)
    { }

    virtual TypeSwitchStmtAST *asTypeSwitchStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class CommClauseAST: public StmtAST
{
public:
    unsigned t_case;
    StmtAST *comm;
    unsigned t_colon;
    StmtListAST *body;

public:
    CommClauseAST(unsigned t_case_ = 0, StmtAST *comm_ = 0, unsigned t_colon_ = 0, StmtListAST *body_ = 0)
        : t_case(t_case_), comm(comm_), t_colon(t_colon_), body(body_)
    { }

    virtual CommClauseAST *asCommClause() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class SelectStmtAST: public StmtAST
{
public:
    unsigned t_select;
    BlockStmtAST *body;

public:
    SelectStmtAST(unsigned t_select_ = 0, BlockStmtAST *body_ = 0)
        : t_select(t_select_), body(body_)
    { }

    virtual SelectStmtAST *asSelectStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class ForStmtAST: public StmtAST
{
public:
    Scope *scope = 0;

public:
    unsigned t_for;
    StmtAST *init;
    ExprAST *cond;
    StmtAST *post;
    BlockStmtAST *body;

public:
    ForStmtAST(unsigned t_for_ = 0, StmtAST *init_ = 0, ExprAST *cond_ = 0, StmtAST *post_ = 0, BlockStmtAST *body_ = 0)
        : t_for(t_for_), init(init_), cond(cond_), post(post_), body(body_)
    { }

    virtual ForStmtAST *asForStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class RangeStmtAST: public StmtAST
{
public:
    Scope *scope = 0;

public:
    unsigned t_for;
    ExprAST *key;
    ExprAST *value;
    unsigned t_assign;
    ExprAST *x;
    BlockStmtAST *body;

public:
    RangeStmtAST(unsigned t_for_ = 0, ExprAST *key_ = 0, ExprAST *value_ = 0, unsigned t_assign_ = 0, ExprAST *x_ = 0, BlockStmtAST *body_ = 0)
        : t_for(t_for_), key(key_), value(value_), t_assign(t_assign_), x(x_), body(body_)
    { }

    virtual RangeStmtAST *asRangeStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

}   // namespace GoTools
