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
#include "ast.h"
#include "astvisitor.h"
#include "resolvecontext.h"
#include "packagetype.h"
#include "gochecksymbols.h"

#include <texteditor/codeassist/assistproposalitem.h>

namespace GoTools {

AST::AST()
{ }

AST::~AST()
{ }

void AST::accept(ASTVisitor *visitor)
{
    if (visitor->preVisit(this))
        accept0(visitor);
    visitor->postVisit(this);
}

unsigned FileAST::firstToken() const
{
    if (doc)
        return doc->firstToken();
    if (t_package)
        return t_package;
    if (packageName)
        return packageName->firstToken();
    if (importDecls)
        return importDecls->firstToken();
    if (decls)
        return decls->firstToken();
    return 0;
}

unsigned FileAST::lastToken() const
{
    if (decls)
        return decls->lastToken();
    if (importDecls)
        return importDecls->lastToken();
    if (packageName)
        return packageName->lastToken();
    if (t_package)
        return t_package;
    if (doc)
        return doc->lastToken();
    return 0;
}

void FileAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(doc, visitor);
        accept(packageName, visitor);
        accept(importDecls, visitor);
        accept(decls, visitor);
    }
    visitor->endVisit(this);
}

unsigned CommentAST::firstToken() const
{
    return t_comment;
}

unsigned CommentAST::lastToken() const
{
    return t_comment;
}

void CommentAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
    }
    visitor->endVisit(this);
}

bool IdentAST::isLookable() const
{ return !ident->isBuiltinTypeIdentifier() && ident != Control::underscoreIdentifier(); }

bool IdentAST::isNewKeyword() const
{
    const char *str = ident->chars();
    return str[0] == '!' && str[1] == '0';
}

bool IdentAST::isMakeKeyword() const
{
    const char *str = ident->chars();
    return str[0] == '!' && str[1] == '1';
}

bool IdentAST::isNewOrMakeKeyword() const
{
    const char *str = ident->chars();
    return str[0] == '!' && (str[1] == '0' || str[1] == '1');
}

unsigned IdentAST::firstToken() const
{
    return t_identifier;
}

unsigned IdentAST::lastToken() const
{
    return t_identifier;
}

ResolvedType IdentAST::resolve(ResolveContext *resolver) const
{
    if (const Type *builtinType = ident->asBuiltinType())
        return ResolvedType(builtinType);

    if (Symbol *s = resolver->currentScope()->lookupMember(this, resolver))
        return s->type(resolver);

    QString idStr(ident->toString());
    return ResolvedType(resolver->packageTypeForAlias(idStr));
}

ResolvedType IdentAST::check(GoCheckSymbols *resolver) const
{
    if (const Type *builtinType = ident->asBuiltinType())
        return ResolvedType(builtinType);

    if (Symbol *s = resolver->currentScope()->lookupMember(this, resolver)) {
        resolver->addUse(this, resolver->kindForSymbol(s));
        return s->type(resolver);
    }

    QString idStr(ident->toString());
    PackageType *pkgTyp = resolver->packageTypeForAlias(idStr);
    if (pkgTyp)
        resolver->addUse(this, GoSemanticHighlighter::Package);
    return ResolvedType(pkgTyp);
}

void IdentAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
    }
    visitor->endVisit(this);
}

unsigned ImportSpecAST::firstToken() const
{
    if (doc)
        return doc->firstToken();
    if (name)
        return name->firstToken();
    if (t_path)
        return t_path;
    if (comment)
        return comment->firstToken();
    return 0;
}

unsigned ImportSpecAST::lastToken() const
{
    if (comment)
        return comment->lastToken();
    if (t_path)
        return t_path;
    if (name)
        return name->lastToken();
    if (doc)
        return doc->lastToken();
    return 0;
}

void ImportSpecAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(doc, visitor);
        accept(name, visitor);
        accept(comment, visitor);
    }
    visitor->endVisit(this);
}

unsigned GenDeclAST::firstToken() const
{
    if (doc)
        return doc->firstToken();
    return t_token;
}

unsigned GenDeclAST::lastToken() const
{
    if (t_rparen)
        return t_rparen;
    if (specs)
        return specs->lastToken();
    if (t_lparen)
        return t_lparen;
    return t_token;
}

void GenDeclAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(doc, visitor);
        accept(specs, visitor);
    }
    visitor->endVisit(this);
}

unsigned VarSpecWithTypeAST::firstToken() const
{
    if (doc)
        return doc->firstToken();
    return names->firstToken();
}

unsigned VarSpecWithTypeAST::lastToken() const
{
    if (comment)
        return comment->lastToken();
    if (type)
        return type->lastToken();
    return names->lastToken();
}

void VarSpecWithTypeAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(doc, visitor);
        accept(names, visitor);
        accept(type, visitor);
        accept(comment, visitor);
    }
    visitor->endVisit(this);
}

unsigned VarSpecWithValuesAST::firstToken() const
{
    if (doc)
        return doc->firstToken();
    return names->firstToken();
}

unsigned VarSpecWithValuesAST::lastToken() const
{
    if (comment)
        return comment->lastToken();
    if (values)
        return values->lastToken();
    return names->lastToken();
}

void VarSpecWithValuesAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(doc, visitor);
        accept(names, visitor);
        accept(values, visitor);
        accept(comment, visitor);
    }
    visitor->endVisit(this);
}

unsigned ConstSpecAST::firstToken() const
{
    if (doc)
        return doc->firstToken();
    return names->firstToken();
}

unsigned ConstSpecAST::lastToken() const
{
    if (comment)
        return comment->lastToken();
    if (values)
        return values->lastToken();
    if (type)
        return type->lastToken();
    return names->lastToken();
}

void ConstSpecAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(doc, visitor);
        accept(names, visitor);
        accept(type, visitor);
        accept(values, visitor);
        accept(comment, visitor);
    }
    visitor->endVisit(this);
}

unsigned SelectorExprAST::firstToken() const
{
    return x->firstToken();
}

unsigned SelectorExprAST::lastToken() const
{
    if (sel)
        return sel->lastToken();
    return x->lastToken();
}

ResolvedType SelectorExprAST::resolve(ResolveContext *resolver) const
{
    if (x && sel && sel->isLookable())
        return x->resolve(resolver).memberAccess(sel, resolver);
    return ResolvedType();
}

ResolvedType SelectorExprAST::check(GoCheckSymbols *resolver) const
{
    if (x && sel && sel->isLookable())
        return x->check(resolver).checkMemberAccess(sel, resolver);

    return ResolvedType();
}

void SelectorExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
        accept(sel, visitor);
    }
    visitor->endVisit(this);
}

unsigned ArrayTypeAST::firstToken() const
{
    return t_lbracket;
}

unsigned ArrayTypeAST::lastToken() const
{
    if (elementType)
        return elementType->lastToken();
    if (t_rbracket)
        return t_rbracket;
    if (len)
        return len->lastToken();
    return t_lbracket;
}

ResolvedType ArrayTypeAST::elementsType(ResolveContext *, int refLvl) const
{ return !refLvl ? elementType->asType() : nullptr; }

QString ArrayTypeAST::describe() const
{ return QStringLiteral("[]") + (elementType ? elementType->describe() : QString()); }

ResolvedType ArrayTypeAST::check(GoCheckSymbols *resolver) const
{
    resolver->accept(len);
    resolver->accept(elementType);
    return ResolvedType(this);
}

void ArrayTypeAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(len, visitor);
        accept(elementType, visitor);
    }
    visitor->endVisit(this);
}

unsigned EllipsisAST::firstToken() const
{
    return t_ellipsis;
}

unsigned EllipsisAST::lastToken() const
{
    return t_ellipsis;
}

void EllipsisAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
    }
    visitor->endVisit(this);
}

unsigned ParenExprAST::firstToken() const
{
    return t_lparen;
}

unsigned ParenExprAST::lastToken() const
{
    if (t_rparen)
        return t_rparen;
    if (x)
        return x->lastToken();
    return t_lparen;
}

ResolvedType ParenExprAST::resolve(ResolveContext *resolver) const
{
    if (x) {
        if (StarExprAST *starExpr = x->asStarExpr()) {
            if (const Type *typeConvertion = tryResolveNamedType(resolver, starExpr->x))
                return ResolvedType(typeConvertion);
        }
        return x->resolve(resolver);
    }

    return ResolvedType();
}

ResolvedType ParenExprAST::check(GoCheckSymbols *resolver) const
{
    if (x) {
        if (StarExprAST *starExpr = x->asStarExpr()) {
            if (const Type *typeConvertion = tryCheckNamedType(resolver, starExpr->x))
                return ResolvedType(typeConvertion);
        }
        return x->check(resolver);
    }

    return ResolvedType();
}

void ParenExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
    }
    visitor->endVisit(this);
}

void BadExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
    }
    visitor->endVisit(this);
}

unsigned BinaryExprAST::firstToken() const
{
    if (x)
        return x->firstToken();
    if (t_op)
        return t_op;
    if (y)
        return y->firstToken();
    return 0;
}

unsigned BinaryExprAST::lastToken() const
{
    if (y)
        return y->lastToken();
    if (t_op)
        return t_op;
    if (x)
        return x->lastToken();
    return 0;
}

ResolvedType BinaryExprAST::resolve(ResolveContext *resolver) const
{ return x ? x->resolve(resolver).applyIntegralOperation(y, resolver) : ResolvedType(); }

ResolvedType BinaryExprAST::check(GoCheckSymbols *resolver) const
{
    resolver->accept(x);
    resolver->accept(y);
    return ResolvedType();
}

void BinaryExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
        accept(y, visitor);
    }
    visitor->endVisit(this);
}

void BinaryPlusExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
        accept(y, visitor);
    }
    visitor->endVisit(this);
}

unsigned UnaryExprAST::firstToken() const
{
    if (t_op)
        return t_op;
    if (x)
        return x->firstToken();
    return 0;
}

unsigned UnaryExprAST::lastToken() const
{
    if (x)
        return x->lastToken();
    if (t_op)
        return t_op;
    return 0;
}

void UnaryExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
    }
    visitor->endVisit(this);
}

ResolvedType ArrowUnaryExprAST::resolve(ResolveContext *resolver) const
{ return x ? x->resolve(resolver).chanValue(resolver) : ResolvedType(); }

ResolvedType ArrowUnaryExprAST::check(GoCheckSymbols *resolver) const
{ return x ? x->check(resolver).chanValue(resolver) : ResolvedType(); }

void ArrowUnaryExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
    }
    visitor->endVisit(this);
}

ResolvedType RefUnaryExprAST::resolve(ResolveContext *resolver) const
{ return x ? x->resolve(resolver).deref() : ResolvedType(); }

ResolvedType RefUnaryExprAST::check(GoCheckSymbols *resolver) const
{ return x ? x->check(resolver).deref() : ResolvedType(); }

void RefUnaryExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
    }
    visitor->endVisit(this);
}

unsigned StarTypeAST::firstToken() const
{
    return t_star;
}

unsigned StarTypeAST::lastToken() const
{
    if (typ)
        return typ->lastToken();
    return t_star;
}

QString StarTypeAST::describe() const
{ return QStringLiteral("*") + (typ ? typ->describe() : QString()); }

ResolvedType StarTypeAST::check(GoCheckSymbols *resolver) const
{ return typ ? typ->check(resolver).deref() : ResolvedType(); }

const TypeSpecAST *StarTypeAST::typeSpec(ResolveContext *resolver) const
{
    if (typ)
        if (!typ->asStarType())
            if (const NamedType *namedTy = typ->asNamedType())
                return namedTy->typeSpec(resolver);
    return nullptr;
}

void StarTypeAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(typ, visitor);
    }
    visitor->endVisit(this);
}

unsigned StarExprAST::firstToken() const
{
    return t_star;
}

unsigned StarExprAST::lastToken() const
{
    if (x)
        return x->lastToken();
    return t_star;
}

ResolvedType StarExprAST::resolve(ResolveContext *resolver) const
{ return x ? x->resolve(resolver).unstar() : ResolvedType(); }

ResolvedType StarExprAST::check(GoCheckSymbols *resolver) const
{ return x ? x->check(resolver).unstar() : ResolvedType(); }

void StarExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
    }
    visitor->endVisit(this);
}

unsigned NewExprAST::firstToken() const
{
    return t_new;
}

unsigned NewExprAST::lastToken() const
{
    if (t_rparen)
        return t_rparen + 1;
    if (typ)
        return typ->lastToken();
    if (t_lparen)
        return t_lparen + 1;
    return t_new + 1;
}

ResolvedType NewExprAST::resolve(ResolveContext *) const
{ return typ ? ResolvedType(typ).deref() : ResolvedType(); }

ResolvedType NewExprAST::check(GoCheckSymbols *resolver) const
{ return typ ? typ->check(resolver).deref() : ResolvedType(); }

void NewExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(typ, visitor);
    }
    visitor->endVisit(this);
}

unsigned MakeExprAST::firstToken() const
{
    return t_make;
}

unsigned MakeExprAST::lastToken() const
{
    if (t_rparen)
        return t_rparen + 1;
    if (list)
        return list->lastToken();
    if (t_lparen)
        return t_lparen + 1;
    return t_make + 1;
}

ResolvedType MakeExprAST::resolve(ResolveContext *resolver) const
{ return list && list->value ? list->value->resolve(resolver) : ResolvedType(); }

ResolvedType MakeExprAST::check(GoCheckSymbols *resolver) const
{
    if (list && list->value) {
        ResolvedType result = list->value->check(resolver);
        for (ExprListAST *x_it = list->next; x_it; x_it = x_it->next)
            resolver->accept(x_it->value);
        return result;
    }

    return ResolvedType();
}

void MakeExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(list, visitor);
    }
    visitor->endVisit(this);
}

QString ChanTypeAST::describe() const
{ return QStringLiteral("chan ") + (value ? value->describe() : QString()); }

ResolvedType ChanTypeAST::resolve(ResolveContext *) const
{ return ResolvedType(this); }

ResolvedType ChanTypeAST::check(GoCheckSymbols *) const
{ return ResolvedType(this); }

void ChanTypeAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(value, visitor);
    }
    visitor->endVisit(this);
}

void BasicLitAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
    }
    visitor->endVisit(this);
}

void StringLitAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
    }
    visitor->endVisit(this);
}

unsigned CompositeLitAST::firstToken() const
{
    if (type)
        return type->firstToken();
    if (t_lbrace)
        return t_lbrace;
    if (elements)
        return elements->firstToken();
    if (t_rbrace)
        return t_rbrace;
    return 0;
}

unsigned CompositeLitAST::lastToken() const
{
    if (t_rbrace)
        return t_rbrace;
    if (elements)
        return elements->lastToken();
    if (t_lbrace)
        return t_lbrace;
    if (type)
        return type->lastToken();
    return 0;
}

ResolvedType CompositeLitAST::resolve(ResolveContext *resolver) const
{ return type ? type->resolve(resolver) : ResolvedType(); }

ResolvedType CompositeLitAST::check(GoCheckSymbols *resolver) const
{
    const Type *typ = resolver->acceptCompositLiteral(this);
    return typ ? ResolvedType(typ) : ResolvedType();
}

void CompositeLitAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(type, visitor);
        accept(elements, visitor);
    }
    visitor->endVisit(this);
}

unsigned KeyValueExprAST::firstToken() const
{
    if (key)
        return key->firstToken();
    if (t_colon)
        return t_colon;
    if (value)
        return value->firstToken();
    return 0;
}

unsigned KeyValueExprAST::lastToken() const
{
    if (value)
        return value->lastToken();
    if (t_colon)
        return t_colon;
    if (key)
        return key->lastToken();
    return 0;
}

void KeyValueExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(key, visitor);
        accept(value, visitor);
    }
    visitor->endVisit(this);
}

unsigned CallExprAST::firstToken() const
{
    if (fun)
        return fun->firstToken();
    if (t_lparen)
        return t_lparen;
    if (args)
        return args->firstToken();
    if (t_ellipsis)
        return t_ellipsis;
    if (t_rparen)
        return t_rparen;
    return 0;
}

unsigned CallExprAST::lastToken() const
{
    if (t_rparen)
        return t_rparen;
    if (t_ellipsis)
        return t_ellipsis;
    if (args)
        return args->lastToken();
    if (t_lparen)
        return t_lparen;
    if (fun)
        return fun->lastToken();
    return 0;
}

ResolvedType CallExprAST::resolve(ResolveContext *resolver) const
{
    if (fun) {
        bool mayBeTypeConvertion = fun->asParenExpr() != nullptr;
        ResolvedType typ = fun->resolve(resolver);
        ResolvedType callTyp = typ.type()->callType(resolver, typ.referenceLevel());
        if (mayBeTypeConvertion && callTyp.type() == Control::unresolvedTupleType())
            return typ;
        return callTyp;
    }

    return Control::unresolvedTupleType();
}

ResolvedType CallExprAST::check(GoCheckSymbols *resolver) const
{
    resolver->accept(args);

    if (fun) {
        bool mayBeTypeConvertion = fun->asParenExpr() != nullptr;
        ResolvedType typ = fun->check(resolver);
        ResolvedType callTyp = typ.type()->callType(resolver, typ.referenceLevel());
        if (mayBeTypeConvertion && callTyp.type() == Control::unresolvedTupleType())
            return typ;
        return callTyp;
    }

    return Control::unresolvedTupleType();
}

void CallExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(fun, visitor);
        accept(args, visitor);
    }
    visitor->endVisit(this);
}

unsigned IndexExprAST::firstToken() const
{
    if (x)
        return x->firstToken();
    if (t_lbracket)
        return t_lbracket;
    if (index)
        return index->firstToken();
    if (t_rbracket)
        return t_rbracket;
    return 0;
}

unsigned IndexExprAST::lastToken() const
{
    if (t_rbracket)
        return t_rbracket;
    if (index)
        return index->lastToken();
    if (t_lbracket)
        return t_lbracket;
    if (x)
        return x->lastToken();
    return 0;
}

ResolvedType IndexExprAST::resolve(ResolveContext *resolver) const
{ return x->resolve(resolver).rangeValue(resolver); }

ResolvedType IndexExprAST::check(GoCheckSymbols *resolver) const
{
    resolver->accept(index);
    return x->check(resolver).rangeValue(resolver);
}

void IndexExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
        accept(index, visitor);
    }
    visitor->endVisit(this);
}

unsigned SliceExprAST::firstToken() const
{
    if (x)
        return x->firstToken();
    if (t_lbracket)
        return t_lbracket;
    if (low)
        return low->firstToken();
    if (high)
        return high->firstToken();
    if (max)
        return max->firstToken();
    if (t_rbracket)
        return t_rbracket;
    return 0;
}

unsigned SliceExprAST::lastToken() const
{
    if (t_rbracket)
        return t_rbracket;
    if (max)
        return max->lastToken();
    if (high)
        return high->lastToken();
    if (low)
        return low->lastToken();
    if (t_lbracket)
        return t_lbracket;
    if (x)
        return x->lastToken();
    return 0;
}

ResolvedType SliceExprAST::resolve(ResolveContext *resolver) const
{ return x->resolve(resolver).rangeValue(resolver); }

ResolvedType SliceExprAST::check(GoCheckSymbols *resolver) const
{
    resolver->accept(low);
    resolver->accept(high);
    resolver->accept(max);
    return x->check(resolver).rangeValue(resolver);
}

void SliceExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
        accept(low, visitor);
        accept(high, visitor);
        accept(max, visitor);
    }
    visitor->endVisit(this);
}

unsigned TypeAssertExprAST::firstToken() const
{
    if (x)
        return x->firstToken();
    if (t_lparen)
        return t_lparen;
    if (typ)
        return typ->firstToken();
    if (t_rparen)
        return t_rparen;
    return 0;
}

unsigned TypeAssertExprAST::lastToken() const
{
    if (t_rparen)
        return t_rparen;
    if (typ)
        return typ->lastToken();
    if (t_lparen)
        return t_lparen;
    if (x)
        return x->lastToken();
    return 0;
}

ResolvedType TypeAssertExprAST::resolve(ResolveContext *) const
{ return ResolvedType(typ); }

ResolvedType TypeAssertExprAST::check(GoCheckSymbols *resolver) const
{
    resolver->accept(x);
    resolver->accept(typ);
    return typ ? ResolvedType(typ) : ResolvedType();
}

void TypeAssertExprAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
        accept(typ, visitor);
    }
    visitor->endVisit(this);
}

void BadDeclAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
    }
    visitor->endVisit(this);
}

unsigned FieldAST::firstToken() const
{
    if (doc)
        return doc->firstToken();
    if (names)
        return names->firstToken();
    if (type)
        return type->firstToken();
    if (tag)
        return tag->firstToken();
    if (comment)
        return comment->firstToken();
    return 0;
}

unsigned FieldAST::lastToken() const
{
    if (comment)
        return comment->lastToken();
    if (tag)
        return tag->lastToken();
    if (type)
        return type->lastToken();
    if (names)
        return names->lastToken();
    if (doc)
        return doc->lastToken();
    return 0;
}

QString FieldAST::describe() const
{
    QString result;

    bool isFirst = true;
    for (DeclIdentListAST *it = names; it; it = it->next) {
        if (DeclIdentAST *decl = it->value) {
            if (isFirst)
                isFirst = false;
            else
                result += QStringLiteral(",");
            result += decl->ident->toString();
        }
    }
    if (type) {
        if (!result.isEmpty())
            result += QStringLiteral(" ");
        result += type->describe();
    }
    return result;
}

void FieldAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(doc, visitor);
        accept(names, visitor);
        accept(type, visitor);
        accept(tag, visitor);
        accept(comment, visitor);
    }
    visitor->endVisit(this);
}

unsigned FieldGroupAST::firstToken() const
{
    if (t_lparen)
        return t_lparen;
    if (fields)
        return fields->firstToken();
    if (t_rparen)
        return t_rparen;
    return 0;
}

unsigned FieldGroupAST::lastToken() const
{
    if (t_rparen)
        return t_rparen;
    if (fields)
        return fields->lastToken();
    if (t_lparen)
        return t_lparen;
    return 0;
}

TupleTypeAST *FieldGroupAST::callType(MemoryPool *pool) const
{
    TupleTypeAST *result = new (pool) TupleTypeAST;
    TypeListAST **type_ptr = &result->types;

    for (FieldListAST *field_it = fields; field_it; field_it = field_it->next) {
        if (FieldAST *field = field_it->value) {
            TypeAST *typ = field->type;
            if (field->names) {
                for (DeclIdentListAST *decl_it = field->names; decl_it; decl_it = decl_it->next) {
                    *type_ptr = new (pool) TypeListAST(typ);
                    type_ptr = &(*type_ptr)->next;
                }
            } else {
                *type_ptr = new (pool) TypeListAST(typ);
                type_ptr = &(*type_ptr)->next;
            }
        }
    }

    return result;
}

QString FieldGroupAST::describe() const
{
    QString result;

    if (t_lparen)
        result += QStringLiteral("(");

    bool isFirst = true;
    for (FieldListAST *it = fields; it; it = it->next) {
        if (FieldAST *field = it->value) {
            if (isFirst)
                isFirst = false;
            else
                result += QStringLiteral(",");
            result += field->describe();
        }
    }

    if (t_rparen)
        result += QStringLiteral(")");

    return result;
}

void FieldGroupAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(fields, visitor);
    }
    visitor->endVisit(this);
}

unsigned StructTypeAST::firstToken() const
{
    if (t_struct)
        return t_struct;
    if (fields)
        return fields->firstToken();
    return 0;
}

unsigned StructTypeAST::lastToken() const
{
    if (fields)
        return fields->lastToken();
    if (t_struct)
        return t_struct;
    return 0;
}

Symbol *StructTypeAST::lookupMember(const IdentAST *ast, ResolveContext *resolver, int refLvl) const
{
    if (fields && (refLvl == 0 || refLvl == -1)) {
        for (FieldListAST *it = fields->fields; it; it = it->next) {
            if (FieldAST *field = it->value) {
                for (DeclIdentListAST *ident_it = field->names; ident_it; ident_it = ident_it->next) {
                    if (DeclIdentAST *ident = ident_it->value) {
                        if (ident->ident->equalTo(ast->ident)) {
                            return ident->symbol;
                        }
                    }
                }
                // embed types
                if (!field->names && field->type) {
                    if (TypeAST *typ = field->type->asType()) {
                        if (StarTypeAST *starType = typ->asStarType()) {
                            typ = starType->typ;
                            if (!typ)
                                continue;
                        }
                        // self embed type
                        if (Symbol *declSymbol = typ->declaration(resolver))
                            if (declSymbol->identifier()->equalTo(ast->ident))
                                return declSymbol;
                        // embed type fields
                        if (Symbol *embedSymbol = typ->lookupMember(ast, resolver))
                            return embedSymbol;
                    }
                }
            }
        }
    }

    return nullptr;
}

void StructTypeAST::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                          ResolveContext *resolver,
                                          int refLvl,
                                          LookupContext::Predicate) const
{
    if (fields && (refLvl == 0 || refLvl == -1)) {
        for (FieldListAST *it = fields->fields; it; it = it->next) {
            if (FieldAST *field = it->value) {
                for (DeclIdentListAST *ident_it = field->names; ident_it; ident_it = ident_it->next) {
                    if (DeclIdentAST *ident = ident_it->value) {
                        TextEditor::AssistProposalItem *item = new TextEditor::AssistProposalItem;;
                        item->setText(ident->ident->toString());
                        item->setIcon(Symbol::icon(Symbol::Var));
                        completions.append(item);
                    }
                }
                // embed types
                if (!field->names && field->type) {
                    if (TypeAST *typ = field->type->asType()) {
                        if (StarTypeAST *starType = typ->asStarType()) {
                            typ = starType->typ;
                            if (!typ)
                                continue;
                        }
                        // self embed type
                        if (Symbol *declSymbol = typ->declaration(resolver)) {
                            TextEditor::AssistProposalItem *item = new TextEditor::AssistProposalItem;;
                            item->setText(declSymbol->identifier()->toString());
                            item->setIcon(Symbol::icon(Symbol::Typ));
                            completions.append(item);
                        }
                        // embed type fields
                        typ->fillMemberCompletions(completions, resolver);
                    }
                }
            }
        }
    }
}

QString StructTypeAST::describe() const
{ return QStringLiteral("struct"); }

ResolvedType StructTypeAST::check(GoCheckSymbols *resolver) const
{
    resolver->accept(fields);
    return ResolvedType(this);
}

void StructTypeAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(fields, visitor);
    }
    visitor->endVisit(this);
}

unsigned FuncTypeAST::firstToken() const
{
    if (t_func)
        return t_func;
    if (params)
        return params->firstToken();
    if (results)
        return results->firstToken();
    return 0;
}

unsigned FuncTypeAST::lastToken() const
{
    if (results)
        return results->lastToken();
    if (params)
        return params->lastToken();
    if (t_func)
        return t_func;
    return 0;
}

ResolvedType FuncTypeAST::callType(ResolveContext *, int refLvl) const
{ return _callType ? (refLvl ? Control::unresolvedTupleType() : _callType) : Control::voidType(); }

int FuncTypeAST::countInTurple() const
{
    int result = 0;
    for (FieldListAST *field_it = results->fields; field_it; field_it = field_it->next)
        result++;
    return result;
}

QString FuncTypeAST::describe() const
{
    QString result = QStringLiteral("func");

    if (params)
        result += params->describe();
    else
        result += QStringLiteral("()");

    if (results)
        result += QStringLiteral(":") + results->describe();

    return result;
}

ResolvedType FuncTypeAST::check(GoCheckSymbols *resolver) const
{
    resolver->accept(params);
    resolver->accept(results);
    return ResolvedType(this);
}

void FuncTypeAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(params, visitor);
        accept(results, visitor);
    }
    visitor->endVisit(this);
}

unsigned InterfaceTypeAST::firstToken() const
{
    if (t_interface)
        return t_interface;
    if (methods)
        return methods->firstToken();
    return 0;
}

unsigned InterfaceTypeAST::lastToken() const
{
    if (methods)
        return methods->lastToken();
    if (t_interface)
        return t_interface;
    return 0;
}

Symbol *InterfaceTypeAST::lookupMember(const IdentAST *ast,
                                       ResolveContext *resolver,
                                       int refLvl) const
{
    if (methods && (refLvl == 0 || refLvl == -1)) {
        for (FieldListAST *it = methods->fields; it; it = it->next) {
            if (FieldAST *field = it->value) {
                for (DeclIdentListAST *ident_it = field->names; ident_it; ident_it = ident_it->next) {
                    if (DeclIdentAST *ident = ident_it->value) {
                        if (ident->ident->equalTo(ast->ident)) {
                            return ident->symbol;
                        }
                    }
                }
                // embed types
                if (!field->names && field->type) {
                    if (TypeAST *typ = field->type->asType())
                        if (Symbol *embedSymbol = typ->lookupMember(ast, resolver))
                            return embedSymbol;
                }
            }
        }
    }

    return nullptr;
}

void InterfaceTypeAST::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                             ResolveContext *resolver,
                                             int refLvl,
                                             LookupContext::Predicate) const
{
    if (methods && (refLvl == 0 || refLvl == -1)) {
        for (FieldListAST *it = methods->fields; it; it = it->next) {
            if (FieldAST *field = it->value) {
                for (DeclIdentListAST *ident_it = field->names; ident_it; ident_it = ident_it->next) {
                    if (DeclIdentAST *ident = ident_it->value) {
                        TextEditor::AssistProposalItem *item = new TextEditor::AssistProposalItem;;
                        item->setText(ident->ident->toString());
                        item->setIcon(Symbol::icon(Symbol::Var));
                        completions.append(item);
                    }
                }
                // embed types
                if (!field->names && field->type) {
                    if (TypeAST *typ = field->type->asType())
                        typ->fillMemberCompletions(completions, resolver);
                }
            }
        }
    }
}

QString InterfaceTypeAST::describe() const
{ return QStringLiteral("interface"); }

ResolvedType InterfaceTypeAST::resolve(ResolveContext *) const
{ return ResolvedType(this); }

ResolvedType InterfaceTypeAST::check(GoCheckSymbols *) const
{ return ResolvedType(this); }

void InterfaceTypeAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(methods, visitor);
    }
    visitor->endVisit(this);
}

unsigned MapTypeAST::firstToken() const
{
    if (t_map)
        return t_map;
    if (key)
        return key->firstToken();
    if (value)
        return value->firstToken();
    return 0;
}

unsigned MapTypeAST::lastToken() const
{
    if (value)
        return value->lastToken();
    if (key)
        return key->lastToken();
    if (t_map)
        return t_map;
    return 0;
}

ResolvedType MapTypeAST::elementsType(ResolveContext *, int refLvl) const
{ return !refLvl ? value->asType() : nullptr; }

ResolvedType MapTypeAST::indexType(ResolveContext *, int refLvl) const
{ return !refLvl ? key->asType() : nullptr; }

QString MapTypeAST::describe() const
{
    return QStringLiteral("map[") + (key ? key->describe() : QString()) +
           QStringLiteral("]") + (value ? value->describe() : QString());
}

ResolvedType MapTypeAST::check(GoCheckSymbols *) const
{ return ResolvedType(this); }

void MapTypeAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(key, visitor);
        accept(value, visitor);
    }
    visitor->endVisit(this);
}

unsigned BlockStmtAST::firstToken() const
{
    if (t_lbrace)
        return t_lbrace;
    if (list)
        return list->firstToken();
    if (t_rbrace)
        return t_rbrace;
    return 0;
}

unsigned BlockStmtAST::lastToken() const
{
    if (t_rbrace)
        return t_rbrace;
    if (list)
        return list->lastToken();
    if (t_lbrace)
        return t_lbrace;
    return 0;
}

void BlockStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(list, visitor);
    }
    visitor->endVisit(this);
}

unsigned FuncDeclAST::firstToken() const
{
    if (doc)
        return doc->firstToken();
    if (recv)
        return recv->firstToken();
    if (name)
        return name->firstToken();
    if (type)
        return type->firstToken();
    if (body)
        return body->firstToken();
    return 0;
}

unsigned FuncDeclAST::lastToken() const
{
    if (body)
        return body->lastToken();
    if (type)
        return type->lastToken();
    if (name)
        return name->lastToken();
    if (recv)
        return recv->lastToken();
    if (doc)
        return doc->lastToken();
    return 0;
}

void FuncDeclAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(doc, visitor);
        accept(recv, visitor);
        accept(name, visitor);
        accept(type, visitor);
        accept(body, visitor);
    }
    visitor->endVisit(this);
}

void BadStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
    }
    visitor->endVisit(this);
}

unsigned LabeledStmtAST::firstToken() const
{
    if (label)
        return label->firstToken();
    if (t_colon)
        return t_colon;
    if (stmt)
        return stmt->firstToken();
    return 0;
}

unsigned LabeledStmtAST::lastToken() const
{
    if (stmt)
        return stmt->lastToken();
    if (t_colon)
        return t_colon;
    if (label)
        return label->lastToken();
    return 0;
}

void LabeledStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(label, visitor);
        accept(stmt, visitor);
    }
    visitor->endVisit(this);
}

unsigned DeclStmtAST::firstToken() const
{
    if (decl)
        return decl->firstToken();
    return 0;
}

unsigned DeclStmtAST::lastToken() const
{
    if (decl)
        return decl->lastToken();
    return 0;
}

void DeclStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(decl, visitor);
    }
    visitor->endVisit(this);
}

unsigned AssignStmtAST::firstToken() const
{
    if (lhs)
        return lhs->firstToken();
    if (t_assign)
        return t_assign;
    if (rhs)
        return rhs->firstToken();
    return 0;
}

unsigned AssignStmtAST::lastToken() const
{
    if (rhs)
        return rhs->lastToken();
    if (t_assign)
        return t_assign;
    if (lhs)
        return lhs->lastToken();
    return 0;
}

void AssignStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(lhs, visitor);
        accept(rhs, visitor);
    }
    visitor->endVisit(this);
}

void DefineStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(lhs, visitor);
        accept(rhs, visitor);
    }
    visitor->endVisit(this);
}

unsigned SendStmtAST::firstToken() const
{
    if (chan)
        return chan->firstToken();
    if (t_arrow)
        return t_arrow;
    if (value)
        return value->firstToken();
    return 0;
}

unsigned SendStmtAST::lastToken() const
{
    if (value)
        return value->lastToken();
    if (t_arrow)
        return t_arrow;
    if (chan)
        return chan->lastToken();
    return 0;
}

void SendStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(chan, visitor);
        accept(value, visitor);
    }
    visitor->endVisit(this);
}

unsigned IncDecStmtAST::firstToken() const
{
    if (x)
        return x->firstToken();
    if (t_pos)
        return t_pos;
    return 0;
}

unsigned IncDecStmtAST::lastToken() const
{
    if (t_pos)
        return t_pos;
    if (x)
        return x->lastToken();
    return 0;
}

void IncDecStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
    }
    visitor->endVisit(this);
}

unsigned ExprStmtAST::firstToken() const
{ return x ? x->firstToken() : 0; }

unsigned ExprStmtAST::lastToken() const
{ return x ? x->lastToken() : 0; }

void ExprStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
    }
    visitor->endVisit(this);
}

unsigned GoStmtAST::firstToken() const
{
    if (t_go)
        return t_go;
    if (call)
        return call->firstToken();
    return 0;
}

unsigned GoStmtAST::lastToken() const
{
    if (call)
        return call->lastToken();
    if (t_go)
        return t_go;
    return 0;
}

void GoStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(call, visitor);
    }
    visitor->endVisit(this);
}

unsigned DeferStmtAST::firstToken() const
{
    if (t_defer)
        return t_defer;
    if (call)
        return call->firstToken();
    return 0;
}

unsigned DeferStmtAST::lastToken() const
{
    if (call)
        return call->lastToken();
    if (t_defer)
        return t_defer;
    return 0;
}

void DeferStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(call, visitor);
    }
    visitor->endVisit(this);
}

unsigned ReturnStmtAST::firstToken() const
{
    if (t_return)
        return t_return;
    if (results)
        return results->firstToken();
    return 0;
}

unsigned ReturnStmtAST::lastToken() const
{
    if (results)
        return results->lastToken();
    if (t_return)
        return t_return;
    return 0;
}

void ReturnStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(results, visitor);
    }
    visitor->endVisit(this);
}

unsigned BranchStmtAST::firstToken() const
{
    if (t_pos)
        return t_pos;
    if (label)
        return label->firstToken();
    return 0;
}

unsigned BranchStmtAST::lastToken() const
{
    if (label)
        return label->lastToken();
    if (t_pos)
        return t_pos;
    return 0;
}

void BranchStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(label, visitor);
    }
    visitor->endVisit(this);
}

unsigned IfStmtAST::firstToken() const
{
    if (t_if)
        return t_if;
    if (init)
        return init->firstToken();
    if (cond)
        return cond->firstToken();
    if (body)
        return body->firstToken();
    if (elseStmt)
        return elseStmt->firstToken();
    return 0;
}

unsigned IfStmtAST::lastToken() const
{
    if (elseStmt)
        return elseStmt->lastToken();
    if (body)
        return body->lastToken();
    if (cond)
        return cond->lastToken();
    if (init)
        return init->lastToken();
    if (t_if)
        return t_if;
    return 0;
}

void IfStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(init, visitor);
        accept(cond, visitor);
        accept(body, visitor);
        accept(elseStmt, visitor);
    }
    visitor->endVisit(this);
}

unsigned CaseClauseAST::firstToken() const
{
    if (t_case)
        return t_case;
    if (list)
        return list->firstToken();
    if (t_colon)
        return t_colon;
    if (body)
        return body->firstToken();
    return 0;
}

unsigned CaseClauseAST::lastToken() const
{
    if (body)
        return body->lastToken();
    if (t_colon)
        return t_colon;
    if (list)
        return list->lastToken();
    if (t_case)
        return t_case;
    return 0;
}

void CaseClauseAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(list, visitor);
        accept(body, visitor);
    }
    visitor->endVisit(this);
}

unsigned SwitchStmtAST::firstToken() const
{
    if (t_switch)
        return t_switch;
    if (init)
        return init->firstToken();
    if (tag)
        return tag->firstToken();
    if (body)
        return body->firstToken();
    return 0;
}

unsigned SwitchStmtAST::lastToken() const
{
    if (body)
        return body->lastToken();
    if (tag)
        return tag->lastToken();
    if (init)
        return init->lastToken();
    if (t_switch)
        return t_switch;
    return 0;
}

void SwitchStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(init, visitor);
        accept(tag, visitor);
        accept(body, visitor);
    }
    visitor->endVisit(this);
}

unsigned TypeSwitchStmtAST::firstToken() const
{
    if (t_switch)
        return t_switch;
    if (init)
        return init->firstToken();
    if (assign)
        return assign->firstToken();
    if (body)
        return body->firstToken();
    return 0;
}

unsigned TypeSwitchStmtAST::lastToken() const
{
    if (body)
        return body->lastToken();
    if (assign)
        return assign->lastToken();
    if (init)
        return init->lastToken();
    if (t_switch)
        return t_switch;
    return 0;
}

void TypeSwitchStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(init, visitor);
        accept(assign, visitor);
        accept(body, visitor);
    }
    visitor->endVisit(this);
}

unsigned CommClauseAST::firstToken() const
{
    if (t_case)
        return t_case;
    if (comm)
        return comm->firstToken();
    if (t_colon)
        return t_colon;
    if (body)
        return body->firstToken();
    return 0;
}

unsigned CommClauseAST::lastToken() const
{
    if (body)
        return body->lastToken();
    if (t_colon)
        return t_colon;
    if (comm)
        return comm->lastToken();
    if (t_case)
        return t_case;
    return 0;
}

void CommClauseAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(comm, visitor);
        accept(body, visitor);
    }
    visitor->endVisit(this);
}

unsigned SelectStmtAST::firstToken() const
{
    if (t_select)
        return t_select;
    if (body)
        return body->firstToken();
    return 0;
}

unsigned SelectStmtAST::lastToken() const
{
    if (body)
        return body->lastToken();
    if (t_select)
        return t_select;
    return 0;
}

void SelectStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(body, visitor);
    }
    visitor->endVisit(this);
}

unsigned ForStmtAST::firstToken() const
{
    if (t_for)
        return t_for;
    if (init)
        return init->firstToken();
    if (cond)
        return cond->firstToken();
    if (post)
        return post->firstToken();
    if (body)
        return body->firstToken();
    return 0;
}

unsigned ForStmtAST::lastToken() const
{
    if (body)
        return body->lastToken();
    if (post)
        return post->lastToken();
    if (cond)
        return cond->lastToken();
    if (init)
        return init->lastToken();
    if (t_for)
        return t_for;
    return 0;
}

void ForStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(init, visitor);
        accept(cond, visitor);
        accept(post, visitor);
        accept(body, visitor);
    }
    visitor->endVisit(this);
}

unsigned RangeStmtAST::firstToken() const
{
    if (t_for)
        return t_for;
    if (key)
        return key->firstToken();
    if (value)
        return value->firstToken();
    if (t_assign)
        return t_assign;
    if (x)
        return x->firstToken();
    if (body)
        return body->firstToken();
    return 0;
}

unsigned RangeStmtAST::lastToken() const
{
    if (body)
        return body->lastToken();
    if (x)
        return x->lastToken();
    if (t_assign)
        return t_assign;
    if (value)
        return value->lastToken();
    if (key)
        return key->lastToken();
    if (t_for)
        return t_for;
    return 0;
}

void RangeStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(key, visitor);
        accept(value, visitor);
        accept(x, visitor);
        accept(body, visitor);
    }
    visitor->endVisit(this);
}

void EmptyStmtAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
    }
    visitor->endVisit(this);
}

unsigned TypeSpecAST::firstToken() const
{
    if (doc)
        return doc->firstToken();
    if (name)
        return name->firstToken();
    if (type)
        return type->firstToken();
    if (comment)
        return comment->firstToken();
    return 0;
}

unsigned TypeSpecAST::lastToken() const
{
    if (comment)
        return comment->lastToken();
    if (type)
        return type->lastToken();
    if (name)
        return name->lastToken();
    if (doc)
        return doc->lastToken();
    return 0;
}

Symbol *TypeSpecAST::lookupMember(const IdentAST *ast, ResolveContext *resolver, int refLvl) const
{
    if (!name || !name->ident || !type)
        return nullptr;

    if (scope && (refLvl == 0 || refLvl == -1)) {
        PackageType *pkg = resolver->fileScopePackageType(scope);
        if (Symbol *method = pkg->lookupMethod(name->ident, ast->ident, resolver))
            return method;
    }

    return type->lookupMember(ast, resolver, refLvl);
}

void TypeSpecAST::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                        ResolveContext *resolver,
                                        int refLvl,
                                        LookupContext::Predicate) const
{
    if (!name || !name->ident || !type)
        return;

    if (scope && (refLvl == 0 || refLvl == -1)) {
        if (PackageType *pkg = resolver->fileScopePackageType(scope))
            pkg->fillMethods(completions, name->ident, resolver);
    }

    type->fillMemberCompletions(completions, resolver, refLvl);
}

ResolvedType TypeSpecAST::indexType(ResolveContext *resolver, int refLvl) const
{ return type ? type->indexType(resolver, refLvl) : nullptr; }

ResolvedType TypeSpecAST::elementsType(ResolveContext *resolver, int refLvl) const
{ return type ? type->elementsType(resolver, refLvl) : nullptr; }

ResolvedType TypeSpecAST::callType(ResolveContext *resolver, int refLvl) const
{ return type ? type->callType(resolver, refLvl) : Control::unresolvedTupleType(); }

ResolvedType TypeSpecAST::chanValueType(ResolveContext *resolver, int refLvl) const
{ return type ? type->chanValueType(resolver, refLvl) : nullptr; }

QString TypeSpecAST::describe() const
{ return name ? name->ident->toString() : QString(); }

Type::BuiltingKind TypeSpecAST::builtinKind(ResolveContext *resolver, int refLvl) const
{ return type ? type->builtinKind(resolver, refLvl) : Other; }

bool TypeSpecAST::hasEmbedOrEqualTo(const TypeSpecAST *spec, ResolveContext *ctx) const
{
    if (spec == this)
        return true;

    if (type) {
        if (StructTypeAST *structType = type->asStructType()) {
            if (structType->fields) {
                for (FieldListAST *fields = structType->fields->fields; fields; fields = fields->next) {
                    if (FieldAST *field = fields->value) {
                        if (!field->names && field->type) {
                            if (const NamedType *namedType = field->type->asNamedType())
                                if (const TypeSpecAST *typeSpec = namedType->typeSpec(ctx))
                                    if (typeSpec->hasEmbedOrEqualTo(spec, ctx))
                                        return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool TypeSpecAST::hasEmbedOrEqualToType(const Type *typ, ResolveContext *ctx) const
{
    if (type) {
        if (type == typ)
            return true;

        if (StructTypeAST *structType = type->asStructType()) {
            if (structType->fields) {
                for (FieldListAST *fields = structType->fields->fields; fields; fields = fields->next) {
                    if (FieldAST *field = fields->value) {
                        if (!field->names && field->type) {
                            if (const NamedType *namedType = field->type->asNamedType())
                                if (const TypeSpecAST *typeSpec = namedType->typeSpec(ctx))
                                    if (typeSpec->hasEmbedOrEqualToType(typ, ctx))
                                        return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

void TypeSpecAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(doc, visitor);
        accept(name, visitor);
        accept(type, visitor);
        accept(comment, visitor);
    }
    visitor->endVisit(this);
}

unsigned FuncLitAST::firstToken() const
{
    if (type)
        return type->firstToken();
    if (body)
        return body->firstToken();
    return 0;
}

unsigned FuncLitAST::lastToken() const
{
    if (body)
        return body->lastToken();
    if (type)
        return type->lastToken();
    return 0;
}

void FuncLitAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(type, visitor);
        accept(body, visitor);
    }
    visitor->endVisit(this);
}

unsigned ParenTypeAST::firstToken() const
{
    return t_lparen;
}

unsigned ParenTypeAST::lastToken() const
{
    if (t_rparen)
        return t_rparen;
    if (x)
        return x->lastToken();
    return t_lparen;
}

QString ParenTypeAST::describe() const
{ return QStringLiteral("(") + (x ? x->describe() : QString()) + QStringLiteral(")"); }

ResolvedType ParenTypeAST::resolve(ResolveContext *resolver) const
{ return x ? x->resolve(resolver) : ResolvedType(); }

ResolvedType ParenTypeAST::check(GoCheckSymbols *resolver) const
{ return x ? x->check(resolver) : ResolvedType(); }

void ParenTypeAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
    }
    visitor->endVisit(this);
}

unsigned PackageTypeAST::firstToken() const
{
    return packageAlias->firstToken();
}

unsigned PackageTypeAST::lastToken() const
{
    return typeName->lastToken();
}

Symbol *PackageTypeAST::lookupMember(const IdentAST *ast, ResolveContext *resolver, int refLvl) const
{
    if (fileScope)
        if (const PackageType *packageLookupcontext = resolver->packageTypeForAlias(packageAlias->ident->toString(), fileScope))
            if (Symbol *symbol = packageLookupcontext->lookupMember(typeName, resolver))
                if (symbol->kind() == Symbol::Typ)
                    return symbol->type(resolver).lookupMember(ast, resolver, refLvl);

    return nullptr;
}

void PackageTypeAST::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                           ResolveContext *resolver,
                                           int refLvl,
                                           LookupContext::Predicate) const
{
    if (fileScope)
        if (const PackageType *packageLookupcontext = resolver->packageTypeForAlias(packageAlias->ident->toString(), fileScope))
            if (Symbol *symbol = packageLookupcontext->lookupMember(typeName, resolver))
                if (symbol->kind() == Symbol::Typ)
                    symbol->type(resolver).fillMemberCompletions(completions, resolver, refLvl);
}

ResolvedType PackageTypeAST::elementsType(ResolveContext *resolver, int refLvl) const
{
    if (fileScope)
        if (PackageType *packageLookupcontext = resolver->packageTypeForAlias(packageAlias->ident->toString(), fileScope))
            if (Symbol *symbol = packageLookupcontext->lookupMember(typeName, resolver))
                if (symbol->kind() == Symbol::Typ)
                    return symbol->type(resolver).rangeValue(resolver, refLvl);

    return nullptr;
}

ResolvedType PackageTypeAST::indexType(ResolveContext *resolver, int refLvl) const
{
    if (fileScope)
        if (PackageType *packageLookupcontext = resolver->packageTypeForAlias(packageAlias->ident->toString(), fileScope))
            if (Symbol *symbol = packageLookupcontext->lookupMember(typeName, resolver))
                if (symbol->kind() == Symbol::Typ)
                    return symbol->type(resolver).rangeKey(resolver, refLvl);

    return nullptr;
}

QString PackageTypeAST::describe() const
{ return packageAlias->ident->toString() + QStringLiteral(".") + (typeName ? typeName->ident->toString() : QString()); }

Type::BuiltingKind PackageTypeAST::builtinKind(ResolveContext *resolver, int refLvl) const
{
    if (fileScope)
        if (PackageType *packageLookupcontext = resolver->packageTypeForAlias(packageAlias->ident->toString(), fileScope))
            if (Symbol *symbol = packageLookupcontext->lookupMember(typeName, resolver))
                if (const TypeSpecAST *typSpec = symbol->typeSpec())
                    return typSpec->builtinKind(resolver, refLvl);

    return Other;
}

ResolvedType PackageTypeAST::check(GoCheckSymbols *resolver) const
{
    if (fileScope) {
        if (PackageType *packageLookupcontext = resolver->packageTypeForAlias(packageAlias->ident->toString(), fileScope)) {
            resolver->addUse(packageAlias, GoSemanticHighlighter::Package);
            if (Symbol *symbol = packageLookupcontext->lookupMember(typeName, resolver)) {
                if (const TypeSpecAST *typSpec = symbol->typeSpec()) {
                    resolver->addUse(typeName, GoSemanticHighlighter::Type);
                    return ResolvedType(typSpec);
                }
            }
        }
    }

    return ResolvedType();
}

Symbol *PackageTypeAST::declaration(ResolveContext *resolver)
{
    if (fileScope) {
        if (PackageType *packageLookupcontext = resolver->packageTypeForAlias(packageAlias->ident->toString(), fileScope)) {
            if (Symbol *symbol = packageLookupcontext->lookupMember(typeName, resolver)) {
                if (symbol->kind() == Symbol::Typ)
                    return symbol;
            }
        }
    }

    return nullptr;
}

const TypeSpecAST *PackageTypeAST::typeSpec(ResolveContext *resolver) const
{
    if (fileScope) {
        if (PackageType *packageLookupcontext = resolver->packageTypeForAlias(packageAlias->ident->toString(), fileScope)) {
            if (Symbol *symbol = packageLookupcontext->lookupMember(typeName, resolver)) {
                if (symbol->kind() == Symbol::Typ)
                    return dynamic_cast<const TypeSpecAST *>(symbol->type(resolver).type());
            }
        }
    }

    return nullptr;
}

void PackageTypeAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(packageAlias, visitor);
        accept(typeName, visitor);
    }
    visitor->endVisit(this);
}

Symbol *TypeIdentAST::lookupMember(const IdentAST *ast, ResolveContext *resolver, int refLvl) const
{
    if (ident->ident == Control::builtinErrorIdentifier() && ast->ident->toString() == QLatin1String("Error"))
        return Control::errorErrorMethod();

    if (Symbol *symbol = usingScope->lookupMember(ident, resolver))
        if (symbol->kind() == Symbol::Typ)
            return symbol->type(resolver).lookupMember(ast, resolver, refLvl);

    return nullptr;
}

void TypeIdentAST::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                         ResolveContext *resolver, int refLvl, LookupContext::Predicate) const
{
    if (ident->ident == Control::builtinErrorIdentifier()) {
        TextEditor::AssistProposalItem *item = new TextEditor::AssistProposalItem;
        item->setText(QLatin1String("Error"));
        item->setIcon(Symbol::icon(Symbol::Mtd));
        completions.append(item);
        return;
    }

    if (Symbol *symbol = usingScope->lookupMember(ident, resolver))
        if (symbol->kind() == Symbol::Typ)
            symbol->type(resolver).fillMemberCompletions(completions, resolver, refLvl);
}

ResolvedType TypeIdentAST::elementsType(ResolveContext *resolver, int refLvl) const
{
    if (Symbol *symbol = usingScope->lookupMember(ident, resolver))
        if (symbol->kind() == Symbol::Typ)
            return symbol->type(resolver).rangeValue(resolver, refLvl);

    return nullptr;
}

ResolvedType TypeIdentAST::indexType(ResolveContext *resolver, int refLvl) const
{
    if (Symbol *symbol = usingScope->lookupMember(ident, resolver))
        if (symbol->kind() == Symbol::Typ)
            return symbol->type(resolver).rangeKey(resolver, refLvl);

    return nullptr;
}

ResolvedType TypeIdentAST::callType(ResolveContext *resolver, int refLvl) const
{
    if (Symbol *symbol = usingScope->lookupMember(ident, resolver))
        if (symbol->kind() == Symbol::Typ)
            return symbol->type(resolver).call(resolver, refLvl);

    return Control::unresolvedTupleType();
}

ResolvedType TypeIdentAST::chanValueType(ResolveContext *resolver, int refLvl) const
{
    if (Symbol *symbol = usingScope->lookupMember(ident, resolver))
        if (symbol->kind() == Symbol::Typ)
            return symbol->type(resolver).chanValue(resolver, refLvl);

    return nullptr;
}

QString TypeIdentAST::describe() const
{ return ident->ident->toString(); }

Type::BuiltingKind TypeIdentAST::builtinKind(ResolveContext *resolver, int refLvl) const
{
    if (Symbol *symbol = usingScope->lookupMember(ident, resolver))
        if (const TypeSpecAST *typeSpec = symbol->typeSpec())
            return typeSpec->builtinKind(resolver, refLvl);
    return Other;
}

ResolvedType TypeIdentAST::check(GoCheckSymbols *resolver) const
{
    resolver->addUse(ident, GoSemanticHighlighter::Type);
    return ResolvedType(this);
}

Symbol *TypeIdentAST::declaration(ResolveContext *resolver)
{
    if (Symbol *symbol = usingScope->lookupMember(ident, resolver))
        if (symbol->kind() == Symbol::Typ)
            return symbol;

    return nullptr;
}

const TypeSpecAST *TypeIdentAST::typeSpec(ResolveContext *resolver) const
{
    if (Symbol *symbol = usingScope->lookupMember(ident, resolver))
        return symbol->typeSpec();

    return nullptr;
}

QString BadTypeAST::describe() const
{ return QStringLiteral("<Something wrong>"); }

void BadTypeAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
    }
    visitor->endVisit(this);
}

void TypeIdentAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(ident, visitor);
    }
    visitor->endVisit(this);
}

void DeclIdentAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
    }
    visitor->endVisit(this);
}

unsigned RhsExprListAST::firstToken() const
{
    if (list)
        return list->firstToken();
    return 0;
}

unsigned RhsExprListAST::lastToken() const
{
    if (list)
        return list->lastToken();
    return 0;
}

ResolvedType RhsExprListAST::type(ResolveContext *resolver, int index)
{
    for (ExprListAST *expr_it = list; expr_it; expr_it = expr_it->next) {
        if (ExprAST *expr = expr_it->value) {
            ResolvedType typ = expr->resolve(resolver);
            int tupleSize = typ.type()->countInTurple();
            if (!tupleSize)     // unresolved tuple
                return ResolvedType();
            if (index < tupleSize)
                return typ.type()->extractFromTuple(index, typ.referenceLevel());
            index -= tupleSize;
        }
    }

    return ResolvedType();
}

ResolvedType RhsExprListAST::resolve(ResolveContext *) const
{
    qDebug() << "RhsExprListAST::resolve: should be unreachable...";
    return ResolvedType();
}

ResolvedType RhsExprListAST::check(GoCheckSymbols *) const
{
    qDebug() << "RhsExprListAST::check: unreachable";
    return ResolvedType();
}

void RhsExprListAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(list, visitor);
    }
    visitor->endVisit(this);
}

unsigned RangeExpAST::firstToken() const
{
    if (x)
        return x->firstToken();
    return 0;
}

unsigned RangeExpAST::lastToken() const
{
    if (x)
        return x->lastToken();
    return 0;
}

ResolvedType RangeExpAST::rangeValueType(ResolveContext *resolver) const
{ return x ? x->resolve(resolver).rangeValue(resolver) : ResolvedType(); }

ResolvedType RangeExpAST::rangeKeyType(ResolveContext *resolver) const
{ return x ? x->resolve(resolver).rangeKeyOrChanType(resolver) : ResolvedType(); }

ResolvedType RangeExpAST::resolve(ResolveContext *) const
{
    qDebug() << "RangeExpAST::resolve should be unreachable";
    return ResolvedType();
}

ResolvedType RangeExpAST::check(GoCheckSymbols *resolver) const
{
    qDebug() << "RangeExpAST::check: unreachable";
    return x ? x->resolve(resolver) : ResolvedType();
}

void RangeExpAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(x, visitor);
    }
    visitor->endVisit(this);
}

unsigned EllipsisTypeAST::firstToken() const
{
    return t_ellipsis;
}

unsigned EllipsisTypeAST::lastToken() const
{
    if (ellipsisElement)
        return ellipsisElement->lastToken();
    return t_ellipsis;
}

ResolvedType EllipsisTypeAST::elementsType(ResolveContext *, int refLvl) const
{ return ellipsisElement && !refLvl ? ellipsisElement->asType() : nullptr; }

QString EllipsisTypeAST::describe() const
{ return QStringLiteral("...") + (ellipsisElement ? ellipsisElement->describe() : QString()); }

void EllipsisTypeAST::accept0(ASTVisitor *visitor)
{
    if (visitor->visit(this)) {
        accept(ellipsisElement, visitor);
    }
    visitor->endVisit(this);
}

ResolvedType TypeAST::resolve(ResolveContext *) const
{ return ResolvedType(this); }

ResolvedType TypeAST::check(GoCheckSymbols *) const
{ return ResolvedType(this); }

Symbol *TupleTypeAST::lookupMember(const IdentAST *ident,
                                   ResolveContext *resolver,
                                   int refLvl) const
{
    if (types && types->value && !types->next && !refLvl)
        return types->value->lookupMember(ident, resolver, refLvl);

    return nullptr;
}

void TupleTypeAST::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                         ResolveContext *resolver,
                                         int refLvl,
                                         LookupContext::Predicate predicate) const
{
    if (types && types->value && !types->next && !refLvl)
        types->value->fillMemberCompletions(completions, resolver, refLvl, predicate);
}

int TupleTypeAST::countInTurple() const
{
    if (!types)
        return 1;   // void type

    int result = 0;
    for (TypeListAST *type_it = types; type_it; type_it = type_it->next)
        ++result;
    return result;
}

ResolvedType TupleTypeAST::extractFromTuple(int index, int refLvl) const
{
    if (!types)
        return index ? ResolvedType() : ResolvedType(Control::voidType()); // void type

    if (refLvl) {
        if (types->value && !types->next && !index)
            return ResolvedType(types->value).setRefLevel(refLvl);
        return ResolvedType();
    }

    int current = 0;
    for (TypeListAST *type_it = types; type_it; type_it = type_it->next, current++)
        if (current == index)
            return ResolvedType(type_it->value);
    return ResolvedType();
}

ResolvedType TupleTypeAST::indexType(ResolveContext *resolver, int refLvl) const
{
    if (types && types->value && !types->next)
        return types->value->indexType(resolver, refLvl);

    return nullptr;
}

ResolvedType TupleTypeAST::elementsType(ResolveContext *resolver, int refLvl) const
{
    if (types && types->value && !types->next)
        return types->value->elementsType(resolver, refLvl);

    return nullptr;
}

ResolvedType TupleTypeAST::chanValueType(ResolveContext *resolver, int refLvl) const
{
    if (types && types->value && !types->next)
        return types->value->chanValueType(resolver, refLvl);

    return nullptr;
}

QString TupleTypeAST::describe() const
{
    QString result = "";

    if (types) {
        result += QStringLiteral("(");
        bool isFirst = true;
        for (TypeListAST *type_it = types; type_it; type_it = type_it->next) {
            if (!isFirst)
                result += QStringLiteral(",");
            else
                isFirst = false;
            result += type_it->value->describe();
        }
        result += QStringLiteral(")");
    }

    return result;
}

Type::BuiltingKind TupleTypeAST::builtinKind(ResolveContext *resolver, int refLvl) const
{
    if (types && types->value && !types->next)
        return types->value->builtinKind(resolver, refLvl);

    return Other;
}

void TupleTypeAST::accept0(ASTVisitor *)
{ }

}   // namespace GoTools
