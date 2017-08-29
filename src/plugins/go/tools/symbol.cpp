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
#include "resolvedtype.h"

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
        case Mtd: return iconProvider->icon(Go::GoIconProvider::Func);
        case Fld: return iconProvider->icon(Go::GoIconProvider::Variable);
        case Arg: return iconProvider->icon(Go::GoIconProvider::Variable);
        default: break; // prevent -Wswitch warning
    }

    return iconProvider->icon(Go::GoIconProvider::Other);
}

ExprAST *VarDecl::declExpr() const
{ return _decl; }

ResolvedType VarDecl::type(ResolveContext *) const
{ return ResolvedType(_decl); }

QString VarDecl::describeType(ResolveContext *) const
{ return _decl->describe(); }

Symbol::Kind VarDecl::kind() const
{ return Var; }

ExprAST *FieldDecl::declExpr() const
{ return _decl; }

ResolvedType FieldDecl::type(ResolveContext *) const
{ return ResolvedType(_decl); }

QString FieldDecl::describeType(ResolveContext *) const
{ return _decl->describe(); }

Symbol::Kind FieldDecl::kind() const
{ return Fld; }

ExprAST *FuncDecl::declExpr() const
{ return _decl; }

ResolvedType FuncDecl::type(ResolveContext *) const
{ return ResolvedType(_decl); }

QString FuncDecl::describeType(ResolveContext *) const
{ return _decl->describe(); }

Symbol::Kind FuncDecl::kind() const
{ return Fun; }

ExprAST *MethodDecl::declExpr() const
{ return _decl; }

ResolvedType MethodDecl::type(ResolveContext *) const
{ return ResolvedType(_decl); }

QString MethodDecl::describeType(ResolveContext *) const
{ return  QString("(%1)%2").arg(_recvIdent->ident->ident->toString()).arg(_decl->describe()); }

Symbol::Kind MethodDecl::kind() const
{ return Mtd; }

TypeIdentAST *MethodDecl::recvIdent() const
{ return _recvIdent; }

ExprAST *ConstDecl::declExpr() const
{ return 0; }

ResolvedType ConstDecl::type(ResolveContext *resolver) const
{ return _value ? _value->resolve(resolver) : ResolvedType(Control::integralBuiltinType()); }

QString ConstDecl::describeType(ResolveContext *) const
{ return QString(); }

Symbol::Kind ConstDecl::kind() const
{ return Con; }

ExprAST *TypeDecl::declExpr() const
{ return _decl->type; }

ResolvedType TypeDecl::type(ResolveContext *) const
{ return ResolvedType(_decl); }

const TypeSpecAST *TypeDecl::typeSpec() const
{ return _decl; }

QString TypeDecl::describeType(ResolveContext *) const
{ return _decl->name->ident->toString() + QStringLiteral(": ") + (_decl->type ? _decl->type->describe() : QString()); }

Symbol::Kind TypeDecl::kind() const
{ return Typ; }

ExprAST *ShortVarDecl::declExpr() const
{ return _decl; }

ResolvedType ShortVarDecl::type(ResolveContext *resolver) const
{ return _decl->type(resolver, _indexInTuple); }

QString ShortVarDecl::describeType(ResolveContext *resolver) const
{ return _decl->type(resolver, _indexInTuple).describe(); }

Symbol::Kind ShortVarDecl::kind() const
{ return Var; }

ExprAST *RangeKeyDecl::declExpr() const
{ return _decl; }

ResolvedType RangeKeyDecl::type(ResolveContext *resolver) const
{ return _decl->rangeKeyType(resolver); }

QString RangeKeyDecl::describeType(ResolveContext *resolver) const
{ return _decl->rangeKeyType(resolver).describe(); }

Symbol::Kind RangeKeyDecl::kind() const
{ return Var; }

ExprAST *RangeValueDecl::declExpr() const
{ return _decl; }

ResolvedType RangeValueDecl::type(ResolveContext *resolver) const
{ return _decl->rangeValueType(resolver); }

QString RangeValueDecl::describeType(ResolveContext *resolver) const
{ return _decl->rangeValueType(resolver).describe(); }

Symbol::Kind RangeValueDecl::kind() const
{ return Var; }

ResolvedType ErrorErrorMethod::type(ResolveContext *) const
{ return ResolvedType(); }

}   // namespace GoTools
