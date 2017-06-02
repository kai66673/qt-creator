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
#include "symbol.h"
#include "scope.h"
#include "ast.h"
#include "goiconprovider.h"

namespace GoTools {

Symbol::Symbol(unsigned tokenIndex, const Identifier *name, Scope *owner)
    : _sourceLocation(tokenIndex)
    , _identifier(name)
    , _owner(owner)
    , _index(0)
    , _next(0)
{ }

Symbol::~Symbol()
{ }

unsigned Symbol::offset(TranslationUnit *translationUnit) const
{ return translationUnit->tokenAt(_sourceLocation).begin(); }

unsigned Symbol::line(TranslationUnit *translationUnit) const
{ return translationUnit->tokenAt(_sourceLocation).line(); }

unsigned Symbol::column(TranslationUnit *translationUnit) const
{ return translationUnit->tokenAt(_sourceLocation).column(); }

unsigned Symbol::index() const
{ return _index; }

Symbol *Symbol::next() const
{ return _next; }

unsigned Symbol::sourceLocation() const
{ return _sourceLocation; }

void Symbol::setSourceLocation(const unsigned &tokenIndex)
{ _sourceLocation = tokenIndex; }

Scope *Symbol::owner() const
{ return _owner; }

FileScope *Symbol::fileScope() const
{ return _owner ? _owner->fileScope() : 0; }

QIcon Symbol::icon(Symbol::Kind kind)
{
    Go::GoIconProvider *iconProvider = Go::GoIconProvider::instance();
    switch (kind) {
        case Bad: return iconProvider->icon(Go::GoIconProvider::Other);
        case Pkg: return iconProvider->icon(Go::GoIconProvider::Package);
        case Con: return iconProvider->icon(Go::GoIconProvider::Const);
        case Typ: return iconProvider->icon(Go::GoIconProvider::Type);
        case Var: return iconProvider->icon(Go::GoIconProvider::Variable);
        case Fun: return iconProvider->icon(Go::GoIconProvider::Func);
        case Fld: return iconProvider->icon(Go::GoIconProvider::Variable);
        case Arg: return iconProvider->icon(Go::GoIconProvider::Variable);
        default: break; // prevent -Wswitch warning
    }

    return iconProvider->icon(Go::GoIconProvider::Other);
}

ExprAST *VarDecl::declExpr() const
{ return _decl; }

Type *VarDecl::type(ExprTypeResolver *)
{ return _decl->asType(); }

QString VarDecl::describeType(ExprTypeResolver *) const
{ return _decl->describe(); }

Symbol::Kind VarDecl::kind() const
{ return Var; }

ExprAST *FuncDecl::declExpr() const
{ return _decl; }

Type *FuncDecl::type(ExprTypeResolver *)
{ return _decl; }

QString FuncDecl::describeType(ExprTypeResolver *) const
{ return _decl->describe(); }

Symbol::Kind FuncDecl::kind() const
{ return Fun; }

ExprAST *ConstDecl::declExpr() const
{ return 0; }

Type *ConstDecl::type(ExprTypeResolver *)
{ return Control::builtinType(); }

QString ConstDecl::describeType(ExprTypeResolver *) const
{ return QString(); }

Symbol::Kind ConstDecl::kind() const
{ return Con; }

ExprAST *TypeDecl::declExpr() const
{ return _decl->type; }

Type *TypeDecl::type(ExprTypeResolver *)
{ return _decl; }

QString TypeDecl::describeType(ExprTypeResolver *) const
{ return _decl->name->ident->toString() + QStringLiteral(": ") + (_decl->type ? _decl->type->describe() : QString()); }

Symbol::Kind TypeDecl::kind() const
{ return Typ; }

ExprAST *ShortVarDecl::declExpr() const
{ return _decl; }

Type *ShortVarDecl::type(ExprTypeResolver *resolver)
{ return _decl->type(resolver, _indexInTuple); }

QString ShortVarDecl::describeType(ExprTypeResolver *resolver) const
{
    if (Type *type = _decl->type(resolver, _indexInTuple))
        return type->describe();
    return QString();
}

Symbol::Kind ShortVarDecl::kind() const
{ return Var; }

ExprAST *RangeKeyDecl::declExpr() const
{ return _decl; }

Type *RangeKeyDecl::type(ExprTypeResolver *resolver)
{ return _decl->type(resolver, 0); }

QString RangeKeyDecl::describeType(ExprTypeResolver *resolver) const
{
    if (Type *type = _decl->type(resolver, 0))
        return type->describe();
    return QString();
}

Symbol::Kind RangeKeyDecl::kind() const
{ return Var; }

ExprAST *RangeValueDecl::declExpr() const
{ return _decl; }

Type *RangeValueDecl::type(ExprTypeResolver *resolver)
{ return _decl->type(resolver, 1); }

QString RangeValueDecl::describeType(ExprTypeResolver *resolver) const
{
    if (Type *type = _decl->type(resolver, 1))
        return type->describe();
    return QString();
}

Symbol::Kind RangeValueDecl::kind() const
{ return Var; }

}   // namespace GoTools
