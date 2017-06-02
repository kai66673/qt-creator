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
class Scope;
class FileScope;
class ExprAST;
class TypeAST;
class TypeSpecAST;
class FuncTypeAST;
class RhsExprListAST;
class ExprTypeResolver;
class RangeExpAST;

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
        Fun,                // function or method
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
    virtual Type *type(ExprTypeResolver *) = 0;
    virtual QString describeType(ExprTypeResolver *) const = 0;
    virtual Kind kind() const = 0;

    static QIcon icon(Kind kind);

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
    virtual Type *type(ExprTypeResolver *) override;
    virtual QString describeType(ExprTypeResolver *) const override;
    virtual Kind kind() const override;

private:
    TypeAST *_decl;
};

class FuncDecl: public Symbol
{
public:
    FuncDecl(unsigned tokenIndex, const Identifier *identifier, FuncTypeAST *decl, Scope *owner)
        : Symbol(tokenIndex, identifier, owner)
        , _decl(decl)
    {}

    virtual ExprAST *declExpr() const override;
    virtual Type *type(ExprTypeResolver *) override;
    virtual QString describeType(ExprTypeResolver *) const override;
    virtual Kind kind() const override;

private:
    FuncTypeAST *_decl;
};

class ConstDecl: public Symbol
{
public:
    ConstDecl(unsigned tokenIndex, const Identifier *identifier, Scope *owner)
        : Symbol(tokenIndex, identifier, owner)
    { }

    virtual ExprAST *declExpr() const override;
    virtual Type *type(ExprTypeResolver *) override;
    virtual QString describeType(ExprTypeResolver *) const override;
    virtual Kind kind() const override;
};

class TypeDecl: public Symbol
{
public:
    TypeDecl(unsigned tokenIndex, const Identifier *identifier, TypeSpecAST *decl, Scope *owner)
        : Symbol(tokenIndex, identifier, owner)
        , _decl(decl)
    { }

    virtual ExprAST *declExpr() const override;
    virtual Type *type(ExprTypeResolver *) override;
    virtual QString describeType(ExprTypeResolver *) const override;
    virtual Kind kind() const override;

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
    virtual Type *type(ExprTypeResolver *resolver) override;
    virtual QString describeType(ExprTypeResolver *resolver) const override;
    virtual Kind kind() const override;

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
    virtual Type *type(ExprTypeResolver *resolver) override;
    virtual QString describeType(ExprTypeResolver *resolver) const override;
    virtual Kind kind() const override;

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
    virtual Type *type(ExprTypeResolver *resolver) override;
    virtual QString describeType(ExprTypeResolver *resolver) const override;
    virtual Kind kind() const override;

private:
    RangeExpAST *_decl;
};

}   // namespace GoTools
