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
#include "exprtyperesolver.h"
#include "ast.h"
#include "gosnapshot.h"

#include <utils/qtcassert.h>

namespace GoTools {

ExprTypeResolver::ExprTypeResolver()
    : m_snapshot(GoPackageCache::instance()->getSnapshot())
    , m_currentIndex(-1)
    , m_currentScope(0)
{ }

ExprTypeResolver::~ExprTypeResolver()
{
    eraseResolvedTypes();
    GoPackageCache::instance()->releaseSnapshot();
}

void ExprTypeResolver::resolve(ExprListAST *list, TupleType *&result)
{
    result = new TupleType;
    m_tuples.append(&result);
    for (ExprListAST *it = list; it; it = it->next) {
        if (ExprAST *expr = it->value) {
            if (Type *typ = expr->asType()) {
                result->appendType(new TypeWithDerefLevel(0, typ));
                continue;
            }

            resolveExpr(result, expr);
        }
    }
}

void ExprTypeResolver::resolve(ExprAST *expr, TupleType *&result)
{
    result = new TupleType;
    m_tuples.append(&result);
    if (Type *typ = expr->asType()) {
        result->appendType(new TypeWithDerefLevel(0, typ));
        return;
    }

    resolveExpr(result, expr);
}

GoSnapshot *ExprTypeResolver::snapshot()
{ return m_snapshot; }

Scope *ExprTypeResolver::currentScope() const
{ return m_currentScope; }

PackageType *ExprTypeResolver::packageTypeForAlias(const QString &alias)
{ return m_snapshot->packageTypeForAlias(m_currentIndex, alias); }

void ExprTypeResolver::eraseResolvedTypes()
{
    for (auto tuple: m_tuples) {
        delete *tuple;
        *tuple = 0;
    }

    m_tuples.clear();
}

void ExprTypeResolver::resolveExpr(TupleType *tuple, ExprAST *x)
{
    int derefLevel = 0;

    if (CallExprAST *callExpr = x->asCallExpr()) {
        // check for...
        if (IdentAST *funcIdent = callExpr->fun->asIdent()) {
            if (funcIdent->isNewKeyword()) {            // new(...) builting function
                if (callExpr->args) {
                    if (Type *typ = resolveExpr(callExpr->args->value, derefLevel)) {
                        tuple->appendType(new TypeWithDerefLevel(derefLevel - 1, typ));
                        return;
                    }
                }
                tuple->appendType(new TypeWithDerefLevel(0, Control::builtinType()));
                return;
            } else if (funcIdent->isMakeKeyword()) {    // make(...) builting function
                if (callExpr->args) {
                    if (Type *typ = resolveExpr(callExpr->args->value, derefLevel)) {
                        tuple->appendType(new TypeWithDerefLevel(derefLevel, typ));
                        return;
                    }
                }
                tuple->appendType(new TypeWithDerefLevel(0, Control::builtinType()));
                return;
            }
        }
        // check for type convertion
        if (ParenExprAST *parenExpr = callExpr->fun->asParenExpr()) {
            if (parenExpr->x) {
                if (StarExprAST *starExpr = parenExpr->x->asStarExpr()) {
                    if (Type *typeConvertion = tryResolveNamedType(this, starExpr->x)) {
                        derefLevel = 0;
                        tuple->appendType(new TypeWithDerefLevel(derefLevel, typeConvertion));
                        return;
                    }
                }
            }
        }
        // common case - function call
        if (Type *context = resolveExpr(callExpr->fun, derefLevel)) {
            context->fillTuple(tuple, this);
            return;
        }

        tuple->appendType(new TypeWithDerefLevel(derefLevel, Control::builtinType()));
        return;
    }

    Type *typ = resolveExpr(x, derefLevel);
    tuple->appendType(new TypeWithDerefLevel(derefLevel, typ));
}

Type *ExprTypeResolver::resolveExpr(ExprAST *x, int &derefLevel)
{
    if (Type *typ = x->asType())
        return typ;

    return x->resolve(this, derefLevel);
}

Type *ExprTypeResolver::resolveCompositExpr(CompositeLitAST *ast)
{
    if (ExprAST *type = ast->type)
        if (TypeAST *typ = type->asType())
            return typ;

    return tryResolveNamedType(this, ast->type);
}

Scope *ExprTypeResolver::switchScope(Scope *scope)
{
    if (! scope)
        return m_currentScope;

    std::swap(m_currentScope, scope);
    return scope;
}

Type *tryResolveNamedType(ExprTypeResolver *resolver, ExprAST *x)
{
    if (x) {
        if (IdentAST *ident = x->asIdent()) {
            if (ident->isLookable()) {
                if (Symbol *s = resolver->currentScope()->lookupMember(ident, resolver)) {
                    if (s->kind() == Symbol::Typ) {
                        return s->type(resolver);
                    }
                }
            }
        } else if (SelectorExprAST *selExpr = x->asSelectorExpr()) {
            if (selExpr->sel->isLookable()) {
                if (IdentAST *packageIdent = selExpr->x->asIdent()) {
                    QString packageAlias(packageIdent->ident->toString());
                    if (PackageType *context = resolver->packageTypeForAlias(packageAlias)) {
                        if (Symbol *s = context->lookupMember(selExpr->sel, resolver)) {
                            if (s->kind() == Symbol::Typ) {
                                return s->type(resolver);
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

}   // namespace GoTools
