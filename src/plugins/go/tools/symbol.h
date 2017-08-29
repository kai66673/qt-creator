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

#include "translationunit.h"

#include <QIcon>
#include <QString>

namespace GoTools {

class Type;
class ResolvedType;
class Scope;
class FileScope;
class ExprAST;
class TypeAST;
class TypeSpecAST;
class FuncTypeAST;
class TypeIdentAST;
class RhsExprListAST;
class ResolveContext;
class RangeExpAST;

class VarDecl;
class FieldDecl;
class FuncDecl;
class MethodDecl;
class ConstDecl;
class TypeDecl;
class ShortVarDecl;
class RangeKeyDecl;
class RangeValueDecl;

class Symbol
{
    Symbol(const Symbol &other);
    void operator =(const Symbol &other);

    friend class SymbolTable;

public:
    enum Kind {
        Bad = 0,            // for error handling
        Pkg,                // package
        Con,                // constant
        Typ,                // type
        Var,                // variable
        Fun,                // function
        Mtd,                // method
        Fld,                // field of struct type
        Arg,                // function argument or result
        Lbl                 // label
    };

    Symbol(unsigned tokenIndex, const Identifier *identifier, Scope *owner);
    virtual ~Symbol();

    inline unsigned offset(TranslationUnit *translationUnit) const;
    inline unsigned line(TranslationUnit *translationUnit) const;
    inline unsigned column(TranslationUnit *translationUnit) const;

    inline const Identifier *identifier() const { return _identifier; }

    unsigned index() const;
    Symbol *next() const;

    unsigned sourceLocation() const;
    void setSourceLocation(const unsigned &tokenIndex);
    Scope *owner() const;
    FileScope *fileScope() const;

    virtual ExprAST *declExpr() const = 0;
    virtual ResolvedType type(ResolveContext *) const = 0;
    virtual const TypeSpecAST *typeSpec() const { return 0; }
    virtual QString describeType(ResolveContext *) const = 0;
    virtual Kind kind() const = 0;

    static QIcon icon(Kind kind);

    virtual VarDecl *asVarDecl() { return 0; }
    virtual FieldDecl *asFieldDecl() { return 0; }
    virtual FuncDecl *asFuncDecl() { return 0; }
    virtual MethodDecl *asMethodDecl() { return 0; }
    virtual ConstDecl *asConstDecl() { return 0; }
    virtual TypeDecl *asTypeDecl() { return 0; }
    virtual ShortVarDecl *asShortVarDecl() { return 0; }
    virtual RangeKeyDecl *asRangeKeyDecl() { return 0; }
    virtual RangeValueDecl *asRangeValueDecl() { return 0; }

private:
    unsigned _sourceLocation;
    const Identifier *_identifier;
    Scope *_owner;

    unsigned _index;
    Symbol *_next;
};

class VarDecl: public Symbol
{
public:
    VarDecl(unsigned tokenIndex, const Identifier *identifier, TypeAST *decl, Scope *owner)
        : Symbol(tokenIndex, identifier, owner)
        , _decl(decl)
    { }

    virtual ExprAST *declExpr() const override;
    virtual ResolvedType type(ResolveContext *) const override;
    virtual QString describeType(ResolveContext *) const override;
    virtual Kind kind() const override;

    virtual VarDecl *asVarDecl() override { return this; }

private:
    TypeAST *_decl;
};

class FieldDecl: public Symbol
{
public:
    FieldDecl(unsigned tokenIndex, const Identifier *identifier, TypeAST *decl, Scope *owner)
        : Symbol(tokenIndex, identifier, owner)
        , _decl(decl)
    { }

    virtual ExprAST *declExpr() const override;
    virtual ResolvedType type(ResolveContext *) const override;
    virtual QString describeType(ResolveContext *) const override;
    virtual Kind kind() const override;

    virtual FieldDecl *asFieldDecl() override { return this; }

private:
    TypeAST *_decl;
};

class FuncDecl: public Symbol
{
public:
    FuncDecl(unsigned tokenIndex, const Identifier *identifier, FuncTypeAST *decl, Scope *owner)
        : Symbol(tokenIndex, identifier, owner)
        , _decl(decl)
    { }

    virtual ExprAST *declExpr() const override;
    virtual ResolvedType type(ResolveContext *) const override;
    virtual QString describeType(ResolveContext *) const override;
    virtual Kind kind() const override;

    virtual FuncDecl *asFuncDecl() override { return this; }

private:
    FuncTypeAST *_decl;
};

class MethodDecl: public Symbol
{
public:
    MethodDecl(unsigned tokenIndex, const Identifier *identifier, TypeIdentAST *recvIdent, FuncTypeAST *decl, Scope *owner)
        : Symbol(tokenIndex, identifier, owner)
        , _decl(decl)
        , _recvIdent(recvIdent)
    { }

    virtual ExprAST *declExpr() const override;
    virtual ResolvedType type(ResolveContext *) const override;
    virtual QString describeType(ResolveContext *) const override;
    virtual Kind kind() const override;

    virtual MethodDecl *asMethodDecl() override { return this; }

    TypeIdentAST *recvIdent() const;

private:
    FuncTypeAST *_decl;
    TypeIdentAST *_recvIdent;
};

class ConstDecl: public Symbol
{
public:
    ConstDecl(unsigned tokenIndex, const Identifier *identifier, ExprAST *value, Scope *owner)
        : Symbol(tokenIndex, identifier, owner)
        , _value(value)
    { }

    virtual ExprAST *declExpr() const override;
    virtual ResolvedType type(ResolveContext *resolver) const override;
    virtual QString describeType(ResolveContext *) const override;
    virtual Kind kind() const override;

    virtual ConstDecl *asConstDecl() override { return this; }

private:
    ExprAST *_value;
};

class TypeDecl: public Symbol
{
public:
    TypeDecl(unsigned tokenIndex, const Identifier *identifier, TypeSpecAST *decl, Scope *owner)
        : Symbol(tokenIndex, identifier, owner)
        , _decl(decl)
    { }

    virtual ExprAST *declExpr() const override;
    virtual ResolvedType type(ResolveContext *) const override;
    virtual const TypeSpecAST *typeSpec() const override;
    virtual QString describeType(ResolveContext *) const override;
    virtual Kind kind() const override;

    virtual TypeDecl *asTypeDecl() override { return this; }

private:
    TypeSpecAST *_decl;
};

class ShortVarDecl: public Symbol
{
public:
    ShortVarDecl(unsigned tokenIndex, const Identifier *identifier, RhsExprListAST *decl, unsigned index, Scope *owner)
        : Symbol(tokenIndex, identifier, owner)
        , _decl(decl)
        , _indexInTuple(index)
    { }

    virtual ExprAST *declExpr() const override;
    virtual ResolvedType type(ResolveContext *resolver) const override;
    virtual QString describeType(ResolveContext *resolver) const override;
    virtual Kind kind() const override;

    virtual ShortVarDecl *asShortVarDecl() override { return this; }

private:
    RhsExprListAST *_decl;
    unsigned _indexInTuple;
};

class RangeKeyDecl: public Symbol
{
public:
    RangeKeyDecl(unsigned tokenIndex, const Identifier *identifier, RangeExpAST *decl, Scope *owner)
        : Symbol(tokenIndex, identifier, owner)
        , _decl(decl)
    { }

    virtual ExprAST *declExpr() const override;
    virtual ResolvedType type(ResolveContext *resolver) const override;
    virtual QString describeType(ResolveContext *resolver) const override;
    virtual Kind kind() const override;

    virtual RangeKeyDecl *asRangeKeyDecl() override { return this; }

private:
    RangeExpAST *_decl;
};

class RangeValueDecl: public Symbol
{
public:
    RangeValueDecl(unsigned tokenIndex, const Identifier *identifier, RangeExpAST *decl, Scope *owner)
        : Symbol(tokenIndex, identifier, owner)
        , _decl(decl)
    { }

    virtual ExprAST *declExpr() const override;
    virtual ResolvedType type(ResolveContext *resolver) const override;
    virtual QString describeType(ResolveContext *resolver) const override;
    virtual Kind kind() const override;

    virtual RangeValueDecl *asRangeValueDecl() override { return this; }

private:
    RangeExpAST *_decl;
};


class ErrorErrorMethod: public Symbol
{
public:
    ErrorErrorMethod() : Symbol(0, 0, 0) { }

    virtual ExprAST *declExpr() const override { return 0; }
    virtual ResolvedType type(ResolveContext *) const override;
    virtual QString describeType(ResolveContext *) const override { return QLatin1String("(error)func():string"); }
    virtual Kind kind() const override { return Mtd; }
};

}   // namespace GoTools
