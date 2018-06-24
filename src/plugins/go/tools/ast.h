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
#include "resolvedtype.h"

namespace GoTools {

class GoCheckSymbols;

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
        _Tp lastValue = nullptr;

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

    virtual FileAST *asFile() { return nullptr; }
    virtual CommentAST *asComment() { return nullptr; }
    virtual SpecAST *asSpec() { return nullptr; }
    virtual FieldAST *asField() { return nullptr; }
    virtual FieldGroupAST *asFieldGroup() { return nullptr; }
    virtual ImportSpecAST *asImportSpec() { return nullptr; }
    virtual VarSpecWithTypeAST *asValueSpecWithType() { return nullptr; }
    virtual VarSpecWithValuesAST *asVarSpecWithValues() { return nullptr; }
    virtual ConstSpecAST *asConstSpec() { return nullptr; }
    virtual TypeSpecAST *asTypeSpec() { return nullptr; }
    virtual DeclAST *asDecl() { return nullptr; }
    virtual BadDeclAST *asBadDecl() { return nullptr; }
    virtual GenDeclAST *asGenDecl() { return nullptr; }
    virtual FuncDeclAST *asFuncDecl() { return nullptr; }
    virtual ExprAST *asExpr() { return nullptr; }
    virtual BadExprAST *asBadExpr() { return nullptr; }
    virtual ParenExprAST *asParenExpr() { return nullptr; }
    virtual RhsExprListAST *asRhsExprList() { return nullptr; }
    virtual RangeExpAST *asRangeExp() { return nullptr; }
    virtual IdentAST *asIdent() { return nullptr; }
    virtual TypeIdentAST *asTypeIdent() { return nullptr; }
    virtual PackageTypeAST *asPackageType() { return nullptr; }
    virtual SelectorExprAST *asSelectorExpr() { return nullptr; }
    virtual IndexExprAST *asIndexExpr() { return nullptr; }
    virtual SliceExprAST *asSliceExpr() { return nullptr; }
    virtual TypeAssertExprAST *asTypeAssertExpr() { return nullptr; }
    virtual EllipsisAST *asEllipsis() { return nullptr; }
    virtual EllipsisTypeAST *asEllipsisType() { return nullptr; }
    virtual CallExprAST *asCallExpr() { return nullptr; }
    virtual StarTypeAST *asStarType() { return nullptr; }
    virtual StarExprAST *asStarExpr() { return nullptr; }
    virtual NewExprAST *asNewExpr() { return nullptr; }
    virtual MakeExprAST *asMakeExpr() { return nullptr; }
    virtual UnaryExprAST *asUnaryExpr() { return nullptr; }
    virtual ArrowUnaryExprAST *asArrowUnaryExpr() { return nullptr; }
    virtual RefUnaryExprAST *asRefUnaryExpr() { return nullptr; }
    virtual BinaryExprAST *asBinaryExpr() { return nullptr; }
    virtual KeyValueExprAST *asKeyValueExpr() { return nullptr; }
    virtual TypeAST *asType() { return nullptr; }
    virtual TupleTypeAST *asTupleType() { return nullptr; }
    virtual BadTypeAST *asBadType() { return nullptr; }
    virtual ParenTypeAST *asParenType() { return nullptr; }
    virtual ArrayTypeAST *asArrayType() { return nullptr; }
    virtual StructTypeAST *asStructType() { return nullptr; }
    virtual ChanTypeAST *asChanType() { return nullptr; }
    virtual FuncTypeAST *asFuncType() { return nullptr; }
    virtual InterfaceTypeAST *asInterfaceType() { return nullptr; }
    virtual MapTypeAST *asMapType() { return nullptr; }
    virtual LitAST *asLit() { return nullptr; }
    virtual BasicLitAST *asBasicLit() { return nullptr; }
    virtual StringLitAST *asStringLit() { return nullptr; }
    virtual FuncLitAST *asFuncLit() { return nullptr; }
    virtual CompositeLitAST *asCompositeLit() { return nullptr; }
    virtual StmtAST *asStmt() { return nullptr; }
    virtual EmptyStmtAST *asEmptyStmt() { return nullptr; }
    virtual BadStmtAST *asBadStmt() { return nullptr; }
    virtual DeclStmtAST *asDeclStmt() { return nullptr; }
    virtual LabeledStmtAST *asLabeledStmt() { return nullptr; }
    virtual ExprStmtAST *asExprStmt() { return nullptr; }
    virtual SendStmtAST *asSendStmt() { return nullptr; }
    virtual IncDecStmtAST *asIncDecStmt() { return nullptr; }
    virtual BlockStmtAST *asBlockStmt() { return nullptr; }
    virtual AssignStmtAST *asAssignStmt() { return nullptr; }
    virtual DefineStmtAST *asDefineStmt() { return nullptr; }
    virtual GoStmtAST *asGoStmt() { return nullptr; }
    virtual DeferStmtAST *asDeferStmt() { return nullptr; }
    virtual ReturnStmtAST *asReturnStmt() { return nullptr; }
    virtual BranchStmtAST *asBranchStmt() { return nullptr; }
    virtual IfStmtAST *asIfStmt() { return nullptr; }
    virtual CaseClauseAST *asCaseClause() { return nullptr; }
    virtual SwitchStmtAST *asSwitchStmt() { return nullptr; }
    virtual TypeSwitchStmtAST *asTypeSwitchStmt() { return nullptr; }
    virtual CommClauseAST *asCommClause() { return nullptr; }
    virtual SelectStmtAST *asSelectStmt() { return nullptr; }
    virtual ForStmtAST *asForStmt() { return nullptr; }
    virtual RangeStmtAST *asRangeStmt() { return nullptr; }

    virtual unsigned firstToken() const = 0;
    virtual unsigned lastToken() const = 0;

    PositionRelation positionRelation(unsigned pos, const std::vector<Token> *tokens) const {
        return pos < tokens->at(firstToken()).begin()
                ? Before
                : (pos <= tokens->at(lastToken()).end()
                   ? Contain
                   : After);
    }

protected:
    virtual void accept0(ASTVisitor *visitor) = 0;
};

class FileAST: public AST
{
public:
    FileScope *scope = nullptr;

public:
    CommentGroupAST *doc;
    unsigned t_package;
    IdentAST *packageName;
    DeclListAST *importDecls;
    DeclListAST *decls;

public:
    FileAST(CommentGroupAST *doc_ = nullptr, unsigned t_package_ = 0, IdentAST *packageName_ = nullptr,
            DeclListAST *importDecls_ = nullptr, DeclListAST *decls_ = nullptr)
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
    FieldAST(CommentGroupAST *doc_ = nullptr, DeclIdentListAST *names_ = nullptr, TypeAST *type_ = nullptr,
             StringLitAST *tag_ = nullptr, CommentGroupAST *comment_ = nullptr)
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
    FieldGroupAST(unsigned t_lparen_ = 0, FieldListAST *fields_ = nullptr, unsigned t_rparen_ = 0)
        : t_lparen(t_lparen_), fields(fields_), t_rparen(t_rparen_)
    { }

    virtual FieldGroupAST *asFieldGroup() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

    TupleTypeAST *callType(MemoryPool *pool) const;

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
    ImportSpecAST(CommentGroupAST *doc_ = nullptr, IdentAST *name_ = nullptr,
                  unsigned t_path_ = 0, CommentGroupAST *comment_ = nullptr)
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
    VarSpecWithTypeAST(CommentGroupAST *doc_ = nullptr, DeclIdentListAST *names_ = nullptr,
                       TypeAST *type_ = nullptr, CommentGroupAST *comment_ = nullptr)
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
    VarSpecWithValuesAST(CommentGroupAST *doc_ = nullptr, DeclIdentListAST *names_ = nullptr,
                         ExprListAST *values_ = nullptr, CommentGroupAST *comment_ = nullptr)
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
    ConstSpecAST(CommentGroupAST *doc_ = nullptr, DeclIdentListAST *names_ = nullptr, TypeAST *type_ = nullptr,
                 ExprListAST *values_ = nullptr, CommentGroupAST *comment_ = nullptr)
        : doc(doc_), names(names_), type(type_), values(values_), comment(comment_)
    { }

    virtual ConstSpecAST *asConstSpec() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

class TypeSpecAST: public SpecAST, public Type, public NamedType
{
public:
    FileScope *scope = nullptr;

public:
    CommentGroupAST *doc;
    DeclIdentAST *name;
    TypeAST *type;
    CommentGroupAST *comment;

public:
    TypeSpecAST(CommentGroupAST *doc_ = nullptr, DeclIdentAST *name_ = nullptr,
                TypeAST *type_ = nullptr, CommentGroupAST *comment_ = nullptr)
        : doc(doc_), name(name_), type(type_), comment(comment_)
    { }

    virtual TypeSpecAST *asTypeSpec() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    // LookupContext implemntation
    virtual Symbol *lookupMember(const IdentAST *ident,
                                 ResolveContext *resolver,
                                 int refLvl = 0) const override;

    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ResolveContext *resolver,
                                       int refLvl = 0,
                                       Predicate = nullptr) const override;


    // Type implementation
    virtual ResolvedType indexType(ResolveContext *resolver, int refLvl = 0) const override;
    virtual ResolvedType elementsType(ResolveContext *resolver, int refLvl = 0) const override;
    virtual ResolvedType callType(ResolveContext *resolver, int refLvl = 0) const override;
    virtual ResolvedType chanValueType(ResolveContext *resolver, int refLvl = 0) const override;

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *resolver, int refLvl = 0) const override;

    virtual const NamedType *asNamedType() const override { return this; }
    virtual const TypeSpecAST *typeSpec(ResolveContext *) const override { return this; }
    bool hasEmbedOrEqualTo(const TypeSpecAST *spec, ResolveContext *ctx) const;
    bool hasEmbedOrEqualToType(const Type *typ, ResolveContext *ctx) const;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
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
    GenDeclAST(CommentGroupAST *doc_ = nullptr, unsigned t_token_ = 0,
               unsigned t_lparen_ = 0, SpecListAST *specs_ = nullptr, unsigned t_rparen_ = 0)
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
    Scope *scope = nullptr;
    Symbol *symbol = nullptr;

public:
    CommentGroupAST *doc;
    FieldGroupAST *recv;
    DeclIdentAST *name;
    FuncTypeAST *type;
    BlockStmtAST *body;

public:
    FuncDeclAST(CommentGroupAST *doc_ = nullptr, FieldGroupAST *recv_ = nullptr, DeclIdentAST *name_ = nullptr,
                FuncTypeAST *type_ = nullptr, BlockStmtAST *body_ = nullptr)
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

    virtual ResolvedType resolve(ResolveContext *) const = 0;
    virtual ResolvedType check(GoCheckSymbols *resolver) const = 0;
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

    virtual BadExprAST *asBadExpr() override { return this; }

    virtual unsigned firstToken() const override { return t_first; }
    virtual unsigned lastToken() const override { return t_last; }

    virtual ResolvedType resolve(ResolveContext *) const override { return ResolvedType(); }
    virtual ResolvedType check(GoCheckSymbols *) const override { return ResolvedType(); }

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class ParenExprAST: public ExprAST
{
public:
    unsigned t_lparen;
    ExprAST *x;
    unsigned t_rparen;

public:
    ParenExprAST(unsigned t_lparen_, ExprAST *x_ = nullptr, unsigned t_rparen_ = 0)
        : t_lparen(t_lparen_), x(x_), t_rparen(t_rparen_)
    { }

    virtual ParenExprAST *asParenExpr() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual ResolvedType resolve(ResolveContext *resolver) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class RhsExprListAST: public ExprAST
{
public:
    ExprListAST *list;

public:
    RhsExprListAST(ExprListAST *list_)
        : list(list_)
    { }

    virtual RhsExprListAST *asRhsExprList() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    ResolvedType type(ResolveContext *resolver, int index);

    virtual ResolvedType resolve(ResolveContext *) const override;
    virtual ResolvedType check(GoCheckSymbols *) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class RangeExpAST: public ExprAST
{
public:
    ExprAST *x;

public:
    RangeExpAST(ExprAST *x_) : x(x_) { }

    virtual RangeExpAST *asRangeExp() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    ResolvedType rangeValueType(ResolveContext *resolver) const;
    ResolvedType rangeKeyType(ResolveContext *resolver) const;

    virtual ResolvedType resolve(ResolveContext *) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
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

    virtual IdentAST *asIdent() override { return this; }

    bool isLookable() const;
    bool isNewKeyword() const;
    bool isMakeKeyword() const;
    bool isNewOrMakeKeyword() const;

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual ResolvedType resolve(ResolveContext *resolver) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class DeclIdentAST: public IdentAST
{
public:
    Symbol *symbol = nullptr;

public:
    DeclIdentAST(unsigned t_identifier_, const Identifier *ident_ = nullptr)
        : IdentAST(t_identifier_, ident_)
    { }

    virtual DeclIdentAST *asDeclIdent() { return this; }

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class TypeAST: public ExprAST, public Type
{
public:
    TypeAST() {}
    virtual TypeAST *asType() override { return this; }

    virtual bool isValidCompositeLiteralType() const = 0;

    // LookupContext implemntation
    virtual Symbol *lookupMember(const IdentAST *, ResolveContext *, int = 0) const override { return nullptr; }
    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &,
                                       ResolveContext *, int = 0, Predicate = nullptr) const override { }

    // Type implementation
    virtual ResolvedType indexType(ResolveContext *, int = 0) const override { return nullptr; }
    virtual ResolvedType elementsType(ResolveContext *, int = 0) const override { return nullptr; }
    virtual ResolvedType chanValueType(ResolveContext *, int = 0) const override { return nullptr; }

    virtual Symbol *declaration(ResolveContext *) { return nullptr; }

    virtual ResolvedType resolve(ResolveContext *) const override;
    virtual ResolvedType check(GoCheckSymbols *) const override;
};

class TupleTypeAST: public TypeAST
{
public:
    TypeListAST *types;

public:
    TupleTypeAST() : types(nullptr) {}
    virtual TupleTypeAST *asTupleType() override { return this; }

    virtual bool isValidCompositeLiteralType() const override { return false; }

    // LookupContext implemntation
    virtual Symbol *lookupMember(const IdentAST *ident,
                                 ResolveContext *resolver,
                                 int refLvl = 0) const override;

    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ResolveContext *resolver,
                                       int refLvl = 0,
                                       Predicate predicate = nullptr) const override;

    // Type implementation
    virtual int countInTurple() const override;
    virtual ResolvedType extractFromTuple(int index, int refLvl = 0) const override;
    virtual ResolvedType indexType(ResolveContext *resolver, int refLvl = 0) const override;
    virtual ResolvedType elementsType(ResolveContext *resolver, int refLvl = 0) const override;
    virtual ResolvedType chanValueType(ResolveContext *resolver, int refLvl = 0) const override;

    virtual unsigned firstToken() const override { return types ? types->firstToken() : 0; }
    virtual unsigned lastToken() const override { return types ? types->lastToken() : 1; }

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *resolver, int refLvl = 0) const override;

protected:
    virtual void accept0(ASTVisitor *) override;
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

    virtual BadTypeAST *asBadType() override { return this; }

    virtual unsigned firstToken() const override { return t_first; }
    virtual unsigned lastToken() const override { return t_last; }

    virtual bool isValidCompositeLiteralType() const override { return true; }

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *, int = 0) const override { return Other; }

    virtual ResolvedType check(GoCheckSymbols *) const override { return ResolvedType(); }

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class ParenTypeAST: public TypeAST
{
public:
    unsigned t_lparen;
    TypeAST *x;
    unsigned t_rparen;

public:
    ParenTypeAST(unsigned t_lparen_, TypeAST *x_ = nullptr, unsigned t_rparen_ = 0)
        : t_lparen(t_lparen_), x(x_), t_rparen(t_rparen_)
    { }

    virtual ParenTypeAST *asParenType() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual bool isValidCompositeLiteralType() const override { return false; }

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *resolver, int refLvl = 0) const override
    { return x ? x->builtinKind(resolver, refLvl) : Other; }

    virtual ResolvedType resolve(ResolveContext *resolver) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class TypeIdentAST: public TypeAST, public NamedType
{
public:
    Scope *usingScope = nullptr;

public:
    IdentAST *ident;

public:
    TypeIdentAST(IdentAST *ident_)
        : ident(ident_)
    { }

    virtual TypeIdentAST *asTypeIdent() override { return this; }
    virtual IdentAST *asIdent() override { return ident; }

    virtual unsigned firstToken() const override { return ident->firstToken(); }
    virtual unsigned lastToken() const override { return ident->lastToken(); }

    virtual bool isValidCompositeLiteralType() const override { return true; }

    // LookupContext implementation
    virtual Symbol *lookupMember(const IdentAST *ast,
                                 ResolveContext *resolver,
                                 int refLvl = 0) const override;

    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ResolveContext *resolver,
                                       int refLvl = 0,
                                       Predicate = nullptr) const override;

    virtual ResolvedType elementsType(ResolveContext *resolver, int refLvl = 0) const override;
    virtual ResolvedType indexType(ResolveContext *resolver, int refLvl = 0) const override;
    virtual ResolvedType callType(ResolveContext *resolver, int refLvl = 0) const override;
    virtual ResolvedType chanValueType(ResolveContext *resolver, int refLvl = 0) const override;

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *resolver, int refLvl = 0) const override;

    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

    virtual Symbol *declaration(ResolveContext *resolver) override;

    virtual const NamedType *asNamedType() const override { return this; }
    virtual const TypeSpecAST *typeSpec(ResolveContext *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class PackageTypeAST: public TypeAST, public NamedType
{
public:
    FileScope *fileScope = nullptr;

public:
    IdentAST *packageAlias;
    unsigned t_dot;
    IdentAST *typeName;

public:
    PackageTypeAST(IdentAST *packageAlias_, unsigned t_dot_, IdentAST *typeName_)
        : packageAlias(packageAlias_), t_dot(t_dot_), typeName(typeName_)
    { }

    virtual PackageTypeAST *asPackageType() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual bool isValidCompositeLiteralType() const override { return true; }

    // LookupContext implementation
    virtual Symbol *lookupMember(const IdentAST *ast,
                                 ResolveContext *resolver,
                                 int refLvl = 0) const override;

    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ResolveContext *resolver,
                                       int refLvl = 0,
                                       Predicate = nullptr) const override;

    virtual ResolvedType elementsType(ResolveContext *resolver, int refLvl = 0) const override;
    virtual ResolvedType indexType(ResolveContext *resolver, int refLvl = 0) const override;

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *resolver, int refLvl = 0) const override;

    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

    virtual Symbol *declaration(ResolveContext *resolver) override;

    virtual const NamedType *asNamedType() const override { return this; }
    virtual const TypeSpecAST *typeSpec(ResolveContext *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class SelectorExprAST: public ExprAST
{
public:
    ExprAST *x;
    IdentAST *sel;

public:
    SelectorExprAST(ExprAST *x_, IdentAST *sel_ = nullptr)
        : x(x_), sel(sel_)
    { }

    virtual SelectorExprAST *asSelectorExpr() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual ResolvedType resolve(ResolveContext *resolver) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class IndexExprAST: public ExprAST
{
public:
    ExprAST *x;
    unsigned t_lbracket;
    ExprAST *index;
    unsigned t_rbracket;

public:
    IndexExprAST(ExprAST *x_ = nullptr, unsigned t_lbracket_ = 0, ExprAST *index_ = nullptr, unsigned t_rbracket_ = 0)
        : x(x_), t_lbracket(t_lbracket_), index(index_), t_rbracket(t_rbracket_)
    { }

    virtual IndexExprAST *asIndexExpr() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual ResolvedType resolve(ResolveContext *resolver) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
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
    SliceExprAST(ExprAST *x_ = nullptr, unsigned t_lbracket_ = 0, ExprAST *low_ = nullptr,
                 ExprAST *high_ = nullptr, ExprAST *max_ = nullptr, unsigned t_rbracket_ = 0)
        : x(x_), t_lbracket(t_lbracket_), low(low_), high(high_), max(max_), t_rbracket(t_rbracket_)
    { }

    virtual SliceExprAST *asSliceExpr() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual ResolvedType resolve(ResolveContext *resolver) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class TypeAssertExprAST: public ExprAST
{
public:
    ExprAST *x;
    unsigned t_lparen;
    TypeAST *typ;
    unsigned t_rparen;

public:
    TypeAssertExprAST(ExprAST *x_ = nullptr, unsigned t_lparen_ = 0, TypeAST *typ_ = nullptr, unsigned t_rparen_ = 0)
        : x(x_), t_lparen(t_lparen_), typ(typ_), t_rparen(t_rparen_)
    { }

    virtual TypeAssertExprAST *asTypeAssertExpr() override { return this; }
    virtual TypeAST *asType() override { return typ ? typ->asType() : nullptr; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual ResolvedType resolve(ResolveContext *) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class EllipsisAST: public ExprAST
{
public:
    unsigned t_ellipsis;

public:
    EllipsisAST(unsigned t_ellipsis_)
        : t_ellipsis(t_ellipsis_)
    { }

    virtual EllipsisAST *asEllipsis() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual ResolvedType resolve(ResolveContext *) const override { return ResolvedType(); }
    virtual ResolvedType check(GoCheckSymbols *) const override { return ResolvedType(); }

protected:
    virtual void accept0(ASTVisitor *visitor) override;
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
    CallExprAST(ExprAST *fun_ = nullptr, unsigned t_lparen_ = 0, ExprListAST *args_ = nullptr, unsigned t_ellipsis_ = 0, unsigned t_rparen_ = 0)
        : fun(fun_), t_lparen(t_lparen_), args(args_), t_ellipsis(t_ellipsis_), t_rparen(t_rparen_)
    { }

    virtual CallExprAST *asCallExpr() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual ResolvedType resolve(ResolveContext *resolver) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class StarTypeAST: public TypeAST, public NamedType
{
public:
    unsigned t_star;
    TypeAST *typ;

public:
    StarTypeAST(unsigned t_star_, TypeAST *x_)
        : t_star(t_star_), typ(x_)
    { }

    virtual StarTypeAST *asStarType() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual bool isValidCompositeLiteralType() const override { return false; }

    // LookupContext implementation
    virtual Symbol *lookupMember(const IdentAST *ident,
                                 ResolveContext *resolver,
                                 int refLvl = 0) const override
    { return typ ? typ->lookupMember(ident, resolver, refLvl - 1) : nullptr; }

    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ResolveContext *resolver,
                                       int refLvl = 0,
                                       Predicate = nullptr) const override
    { if (typ) typ->fillMemberCompletions(completions, resolver, refLvl - 1); }

    // Type implementation
    virtual ResolvedType callType(ResolveContext *resolver, int refLvl = 0) const override
    { return typ ? typ->callType(resolver, refLvl - 1) : nullptr; }

    virtual ResolvedType elementsType(ResolveContext *resolver, int refLvl = 0) const override
    { return typ ? typ->elementsType(resolver, refLvl - 1) : nullptr; }

    virtual ResolvedType indexType(ResolveContext *resolver, int refLvl = 0) const override
    { return typ ? typ->indexType(resolver, refLvl - 1) : nullptr; }

    virtual ResolvedType chanValueType(ResolveContext *resolver, int refLvl = 0) const override
    { return typ ? typ->chanValueType(resolver, refLvl - 1) : nullptr; }

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *resolver, int refLvl = 0) const override
    { return typ ? typ->builtinKind(resolver, refLvl - 1) : Other; }

    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

    virtual const NamedType *asNamedType() const override { return this; }
    virtual const TypeSpecAST *typeSpec(ResolveContext *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
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

    virtual StarExprAST *asStarExpr() override { return this; }
    virtual TypeAST *asType() override { return x ? x->asType() : nullptr; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual bool isValidCompositeLiteralType() const { return false; }

    virtual ResolvedType resolve(ResolveContext *resolver) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class NewExprAST: public ExprAST
{
public:
    unsigned t_new;
    unsigned t_lparen;
    TypeAST *typ;
    unsigned t_rparen;

public:
    NewExprAST(unsigned t_new_, unsigned t_lparen_ = 0, TypeAST *typ_ = nullptr, unsigned t_rparen_ = 0)
        : t_new(t_new_), t_lparen(t_lparen_), typ(typ_), t_rparen(t_rparen_)
    { }

    virtual NewExprAST *asNewExpr() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual bool isValidCompositeLiteralType() const { return false; }

    virtual ResolvedType resolve(ResolveContext *) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class MakeExprAST: public ExprAST
{
public:
    unsigned t_make;
    unsigned t_lparen;
    ExprListAST *list;
    unsigned t_rparen;

public:
    MakeExprAST(unsigned t_make_, unsigned t_lparen_ = 0, ExprListAST *list_ = nullptr, unsigned t_rparen_ = 0)
        : t_make(t_make_), t_lparen(t_lparen_), list(list_), t_rparen(t_rparen_)
    { }

    virtual MakeExprAST *asMakeExpr() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual bool isValidCompositeLiteralType() const { return false; }

    virtual ResolvedType resolve(ResolveContext *resolver) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class UnaryExprAST: public ExprAST
{
public:
    unsigned t_op;
    ExprAST *x;

public:
    UnaryExprAST(unsigned t_op_ = 0, ExprAST *x_ = nullptr)
        : t_op(t_op_), x(x_)
    { }

    virtual UnaryExprAST *asUnaryExpr() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual ResolvedType resolve(ResolveContext *resolver) const override
    { return x ? x->resolve(resolver) : ResolvedType(); }
    virtual ResolvedType check(GoCheckSymbols *resolver) const override
    { return x ? x->check(resolver) : ResolvedType(); }

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class ArrowUnaryExprAST: public UnaryExprAST
{
public:
    ArrowUnaryExprAST(unsigned t_op_ = 0, ExprAST *x_ = nullptr)
        : UnaryExprAST(t_op_, x_)
    { }

    virtual ArrowUnaryExprAST *asArrowUnaryExpr() override { return this; }

    virtual ResolvedType resolve(ResolveContext *resolver) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class RefUnaryExprAST: public UnaryExprAST
{
public:
    RefUnaryExprAST(unsigned t_op_ = 0, ExprAST *x_ = nullptr)
        : UnaryExprAST(t_op_, x_)
    { }

    virtual RefUnaryExprAST *asRefUnaryExpr() override { return this; }

    virtual ResolvedType resolve(ResolveContext *resolver) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class BinaryExprAST: public ExprAST
{
public:
    ExprAST *x;
    unsigned t_op;
    ExprAST *y;

public:
    BinaryExprAST(ExprAST *x_ = nullptr, unsigned t_op_ = 0, ExprAST *y_ = nullptr)
        : x(x_), t_op(t_op_), y(y_)
    { }

    virtual BinaryExprAST *asBinaryExpr() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual ResolvedType resolve(ResolveContext *resolver) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class BinaryPlusExprAST: public BinaryExprAST
{
public:
    BinaryPlusExprAST(ExprAST *x_ = nullptr, unsigned t_op_ = 0, ExprAST *y_ = nullptr)
        : BinaryExprAST(x_, t_op_, y_)
    { }

    virtual ResolvedType resolve(ResolveContext *resolver) const override
    { return x ? x->resolve(resolver).applyPlusOperation(y, resolver) : ResolvedType(); }

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class KeyValueExprAST: public ExprAST
{
public:
    ExprAST *key;
    unsigned t_colon;
    ExprAST *value;

public:
    KeyValueExprAST(ExprAST *key_ = nullptr, unsigned t_colon_ = 0, ExprAST *value_ = nullptr)
        : key(key_), t_colon(t_colon_), value(value_)
    { }

    virtual KeyValueExprAST *asKeyValueExpr() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual ResolvedType resolve(ResolveContext *) const override
    { return ResolvedType(); }
    virtual ResolvedType check(GoCheckSymbols *) const override
    { return ResolvedType(); }

protected:
    virtual void accept0(ASTVisitor *visitor) override;
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

    virtual EllipsisTypeAST *asEllipsisType() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual bool isValidCompositeLiteralType() const override { return false; }

    virtual ResolvedType elementsType(ResolveContext *, int refLvl = 0) const override;

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *, int = 0) const override { return Other; }

    virtual ResolvedType resolve(ResolveContext *) const override { return ResolvedType(); }
    virtual ResolvedType check(GoCheckSymbols *) const override { return ResolvedType(); }

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class ArrayTypeAST: public TypeAST
{
public:
    unsigned t_lbracket;
    ExprAST *len;
    unsigned t_rbracket;
    TypeAST *elementType;

public:
    ArrayTypeAST(unsigned t_lbracket_, ExprAST *len_ = nullptr, unsigned t_rbracket_ = 0, TypeAST *elementType_ = nullptr)
        : t_lbracket(t_lbracket_), len(len_), t_rbracket(t_rbracket_), elementType(elementType_)
    { }

    virtual ArrayTypeAST *asArrayType() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual bool isValidCompositeLiteralType() const override { return true; }

    virtual ResolvedType elementsType(ResolveContext *, int refLvl = 0) const override;

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *, int = 0) const override { return Other; }

    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
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

    virtual ChanTypeAST *asChanType() override { return this; }

    virtual unsigned firstToken() const override { return t_chan; }
    virtual unsigned lastToken() const override { return value->lastToken(); }

    virtual bool isValidCompositeLiteralType() const override { return false; }

    virtual ResolvedType chanValueType(ResolveContext *, int refLvl = 0) const override
    { return !refLvl ? value : nullptr; }

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *, int = 0) const override { return Other; }

    virtual ResolvedType resolve(ResolveContext *) const override;
    virtual ResolvedType check(GoCheckSymbols *) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class FuncTypeAST: public TypeAST
{
public:
    unsigned t_func;
    FieldGroupAST *params;
    FieldGroupAST *results;

    TupleTypeAST *_callType;     // 0 is void type

public:
    FuncTypeAST(unsigned t_func_ = 0, FieldGroupAST *params_ = nullptr,
                FieldGroupAST *results_ = nullptr, TupleTypeAST *callType_ = nullptr)
        : t_func(t_func_), params(params_), results(results_), _callType(callType_)
    { }

    virtual FuncTypeAST *asFuncType() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual bool isValidCompositeLiteralType() const override { return false; }

    virtual ResolvedType callType(ResolveContext *, int refLvl = 0) const override;
    virtual int countInTurple() const override;

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *, int = 0) const override { return Other; }

    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class StructTypeAST: public TypeAST
{
public:
    unsigned t_struct;
    FieldGroupAST *fields;

public:
    StructTypeAST(unsigned t_struct_ = 0, FieldGroupAST *fields_ = nullptr)
        : t_struct(t_struct_), fields(fields_)
    { }

    virtual StructTypeAST *asStructType() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual bool isValidCompositeLiteralType() const override { return true; }

    // LookupContext implementation
    virtual Symbol *lookupMember(const IdentAST *ast,
                                 ResolveContext *resolver,
                                 int refLvl = 0) const override;

    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ResolveContext *resolver,
                                       int refLvl = 0,
                                       Predicate = nullptr) const override;

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *, int = 0) const override { return Other; }

    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class InterfaceTypeAST: public TypeAST
{
public:
    bool incomplete;

public:
    unsigned t_interface;
    FieldGroupAST *methods;

public:
    InterfaceTypeAST(unsigned t_interface_ = 0, FieldGroupAST *methods_ = nullptr)
        : incomplete(true) , t_interface(t_interface_), methods(methods_)
    { }

    virtual InterfaceTypeAST *asInterfaceType() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual bool isValidCompositeLiteralType() const override { return false; }

    // LookupContext implementation
    virtual Symbol *lookupMember(const IdentAST *ast,
                                 ResolveContext *resolver,
                                 int refLvl) const override;

    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ResolveContext *resolver,
                                       int refLvl = 0,
                                       Predicate = nullptr) const override;

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *, int = 0) const override { return Other; }

    virtual ResolvedType resolve(ResolveContext *) const override;
    virtual ResolvedType check(GoCheckSymbols *) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class MapTypeAST: public TypeAST
{
public:
    unsigned t_map;
    TypeAST *key;
    TypeAST *value;

public:
    MapTypeAST(unsigned t_map_ = 0, TypeAST *key_ = nullptr, TypeAST *value_ = nullptr)
        : t_map(t_map_), key(key_), value(value_)
    { }

    virtual MapTypeAST *asMapType() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual bool isValidCompositeLiteralType() const override { return true; }

    virtual ResolvedType elementsType(ResolveContext *, int refLvl = 0) const override;
    virtual ResolvedType indexType(ResolveContext *, int refLvl = 0) const override;

    virtual QString describe() const override;
    virtual BuiltingKind builtinKind(ResolveContext *, int = 0) const override { return Other; }

    virtual ResolvedType check(GoCheckSymbols *) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
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

    virtual BasicLitAST *asBasicLit() override { return this; }

    virtual unsigned firstToken() const override { return t_value; }
    virtual unsigned lastToken() const override { return t_value; }

    virtual ResolvedType resolve(ResolveContext *) const override
    { return ResolvedType(Control::integralBuiltinType()); }
    virtual ResolvedType check(GoCheckSymbols *) const override
    { return ResolvedType(); }

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class StringLitAST: public BasicLitAST
{
public:
    StringLitAST(unsigned t_value_)
        : BasicLitAST(t_value_)
    { }

    virtual StringLitAST *asStringLit() override { return this; }

    virtual ResolvedType resolve(ResolveContext *) const override
    { return ResolvedType(Control::stringBuiltingType()); }
    virtual ResolvedType check(GoCheckSymbols *) const override
    { return ResolvedType(); }

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class FuncLitAST: public LitAST
{
public:
    FuncTypeAST *type;
    BlockStmtAST *body;

public:
    FuncLitAST(FuncTypeAST *type_ = nullptr, BlockStmtAST *body_ = nullptr)
        : type(type_), body(body_)
    { }

    virtual FuncLitAST *asFuncLit() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual ResolvedType resolve(ResolveContext *) const override { return ResolvedType(type); }
    virtual ResolvedType check(GoCheckSymbols *) const override
    { return type ? ResolvedType(type) : ResolvedType(); }

protected:
    virtual void accept0(ASTVisitor *visitor) override;
};

class CompositeLitAST: public LitAST
{
public:
    ExprAST *type;
    unsigned t_lbrace;
    ExprListAST *elements;
    unsigned t_rbrace;

public:
    CompositeLitAST(ExprAST *type_ = nullptr, unsigned t_lbrace_ = 0,
                    ExprListAST *elements_ = nullptr, unsigned t_rbrace_ = 0)
        : type(type_), t_lbrace(t_lbrace_), elements(elements_), t_rbrace(t_rbrace_)
    { }

    virtual CompositeLitAST *asCompositeLit() override { return this; }

    virtual unsigned firstToken() const override;
    virtual unsigned lastToken() const override;

    virtual ResolvedType resolve(ResolveContext *resolver) const override;
    virtual ResolvedType check(GoCheckSymbols *resolver) const override;

protected:
    virtual void accept0(ASTVisitor *visitor) override;
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
    DeclStmtAST(DeclAST *decl_ = nullptr)
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
    LabeledStmtAST(IdentAST *label_ = nullptr, unsigned t_colon_ = 0, StmtAST *stmt_ = nullptr)
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
    ExprStmtAST(ExprAST *x_ = nullptr)
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
    SendStmtAST(ExprAST *chan_ = nullptr, unsigned t_arrow_ = 0, ExprAST *value_ = nullptr)
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
    IncDecStmtAST(ExprAST *x_ = nullptr, unsigned t_pos_ = 0)
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
    Scope *scope = nullptr;

public:
    unsigned t_lbrace;
    StmtListAST *list;
    unsigned t_rbrace;

public:
    BlockStmtAST(unsigned t_lbrace_ = 0, StmtListAST *list_ = nullptr, unsigned t_rbrace_ = 0)
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
    AssignStmtAST(ExprListAST *lhs_ = nullptr, unsigned t_assign_ = 0, ExprListAST *rhs_ = nullptr)
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
    DefineStmtAST(ExprListAST *lhs_ = nullptr, unsigned t_assign_ = 0, ExprListAST *rhs_ = nullptr)
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
    GoStmtAST(unsigned t_go_ = 0, CallExprAST *call_ = nullptr)
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
    DeferStmtAST(unsigned t_defer_ = 0, CallExprAST *call_ = nullptr)
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
    ReturnStmtAST(unsigned t_return_ = 0, ExprListAST *results_ = nullptr)
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
    BranchStmtAST(unsigned t_pos_ = 0, IdentAST *label_ = nullptr)
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
    Scope *scope = nullptr;

public:
    unsigned t_if;
    StmtAST *init;
    ExprAST *cond;
    BlockStmtAST *body;
    StmtAST *elseStmt;

public:
    IfStmtAST(unsigned t_if_ = 0, StmtAST *init_ = nullptr, ExprAST *cond_ = nullptr,
              BlockStmtAST *body_ = nullptr, StmtAST *elseStmt_ = nullptr)
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
    Scope *scope = nullptr;

public:
    unsigned t_case;
    ExprListAST *list;
    unsigned t_colon;
    StmtListAST *body;

public:
    CaseClauseAST(unsigned t_case_ = 0, ExprListAST *list_ = nullptr, unsigned t_colon_ = 0, StmtListAST *body_ = nullptr)
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
    Scope *scope = nullptr;

public:
    unsigned t_switch;
    StmtAST *init;
    ExprAST *tag;
    BlockStmtAST *body;

public:
    SwitchStmtAST(unsigned t_switch_ = 0, StmtAST *init_ = nullptr, ExprAST *tag_ = nullptr, BlockStmtAST *body_ = nullptr)
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
    Scope *scope = nullptr;

public:
    unsigned t_switch;
    StmtAST *init;
    StmtAST *assign;
    BlockStmtAST *body;

public:
    TypeSwitchStmtAST(unsigned t_switch_ = 0, StmtAST *init_ = nullptr, StmtAST *assign_ = nullptr, BlockStmtAST *body_ = nullptr)
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
    CommClauseAST(unsigned t_case_ = 0, StmtAST *comm_ = nullptr, unsigned t_colon_ = 0, StmtListAST *body_ = nullptr)
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
    SelectStmtAST(unsigned t_select_ = 0, BlockStmtAST *body_ = nullptr)
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
    Scope *scope = nullptr;

public:
    unsigned t_for;
    StmtAST *init;
    ExprAST *cond;
    StmtAST *post;
    BlockStmtAST *body;

public:
    ForStmtAST(unsigned t_for_ = 0, StmtAST *init_ = nullptr, ExprAST *cond_ = nullptr,
               StmtAST *post_ = nullptr, BlockStmtAST *body_ = nullptr)
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
    Scope *scope = nullptr;

public:
    unsigned t_for;
    ExprAST *key;
    ExprAST *value;
    unsigned t_assign;
    ExprAST *x;
    BlockStmtAST *body;

public:
    RangeStmtAST(unsigned t_for_ = 0, ExprAST *key_ = nullptr, ExprAST *value_ = nullptr,
                 unsigned t_assign_ = 0, ExprAST *x_ = nullptr, BlockStmtAST *body_ = nullptr)
        : t_for(t_for_), key(key_), value(value_), t_assign(t_assign_), x(x_), body(body_)
    { }

    virtual RangeStmtAST *asRangeStmt() { return this; }

    virtual unsigned firstToken() const;
    virtual unsigned lastToken() const;

protected:
    virtual void accept0(ASTVisitor *visitor);
};

}   // namespace GoTools
