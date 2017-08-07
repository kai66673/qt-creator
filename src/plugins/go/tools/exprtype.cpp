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
#include "exprtype.h"
#include "ast.h"
#include "gochecksymbols.h"

namespace GoTools {

ExprType::ExprType(const Type *typ, int derefLevel)
{ push_back({derefLevel, typ ? typ : Control::unresolvedType()}); }

ExprType::ExprType(const QList<const Type *> &types)
{
    for (const Type *type: types)
        push_back({type->refLevel(), type->baseType()});

    if (empty())
        push_back({0, Control::voidType()});
}

const Type *ExprType::type() const
{ return size() == 1 ? first().second : Control::unresolvedType(); }

const Type *ExprType::type(int indexInTuple) const
{
    return indexInTuple >= 0 && indexInTuple < size()
            ? at(indexInTuple).second
            : Control::unresolvedType();
}

const Type *ExprType::typeForMemberAccess() const
{
    if (size() != 1)
        return 0;

    auto &firstItem = first();
    return firstItem.first == 0 || firstItem.first == -1 ? firstItem.second : 0;
}

const Type *ExprType::typeForDirectAccess() const
{
    if (size() != 1)
        return 0;

    auto &firstItem = first();
    return firstItem.first == 0 ? firstItem.second : 0;
}

Symbol *ExprType::lookupMember(const IdentAST *ident, ResolveContext *resolver)
{
    if (ident && ident->isLookable() && size() == 1) {
        auto &firstItem = first();
        if (firstItem.first == 0 || firstItem.first == -1)
            return firstItem.second->lookupMember(ident, resolver);
    }

    return 0;
}

void ExprType::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions, ResolveContext *resolver)
{
    if (size() == 1) {
        auto &firstItem = first();
        if (firstItem.first == 0 || firstItem.first == -1)
            firstItem.second->fillMemberCompletions(completions, resolver);
    }
}

ExprType &ExprType::unresolve()
{
    clear();
    push_back({0, Control::unresolvedType()});
    return *this;
}

ExprType &ExprType::applyIntegralOperation(ExprAST *x, ResolveContext *resolver)
{
    if (x && size() == 1 && first().first == 0) {
        const Type *typ = first().second;
        if (typ->isIntegral(resolver)) {
            typ = x->resolveExprType(resolver).type();
            if (typ->isIntegral(resolver))
                return *this;
        }
    }

    return unresolve();
}

ExprType &ExprType::applyPlusOperation(ExprAST *x, ResolveContext *resolver)
{
    if (x && size() == 1 && first().first == 0) {
        const Type *typ = first().second;
        if (typ->isString(resolver)) {
            typ = x->resolveExprType(resolver).type();
            if (typ->isString(resolver))
                return *this;
        } else if (typ->isIntegral(resolver)) {
            typ = x->resolveExprType(resolver).type();
            if (typ->isIntegral(resolver))
                return *this;
        }
    }

    return unresolve();
}

bool ExprType::applyCommaJoin(ExprAST *x, ResolveContext *resolver)
{
    if (last().second == Control::unresolvedTupleType())
        return false;
    if (x)
        for (const auto &it: x->resolveExprType(resolver))
            if (it.second != Control::voidType())
                push_back(it);
    return true;
}

ExprType &ExprType::unstar()
{
    if (size() == 1) {
        ++first().first;
        return *this;
    }

    return unresolve();
}

ExprType &ExprType::deref()
{
    if (size() == 1) {
        --first().first;
        return *this;
    }

    return unresolve();
}

ExprType &ExprType::switchTo(const Type *typ)
{
    if (!typ)
        return unresolve();

    auto &exprtype = first();
    exprtype.first = typ->refLevel();
    exprtype.second = typ->baseType();
    return *this;
}

ExprType &ExprType::memberAccess(IdentAST *ident, ResolveContext *resolver)
{
    if (size() == 1) {
        auto &exprtype = first();
        int refLevel = exprtype.first;
        if (refLevel == 0 || refLevel == -1) {
            if (Symbol *s = exprtype.second->lookupMember(ident, resolver))
                return switchTo(s->type(resolver));
        }
    }

    return unresolve();
}

ExprType &ExprType::checkMemberAccess(IdentAST *ident, GoCheckSymbols *resolver)
{
    if (size() == 1) {
        auto &exprtype = first();
        int refLevel = exprtype.first;
        if (refLevel == 0 || refLevel == -1) {
            if (Symbol *s = exprtype.second->lookupMember(ident, resolver)) {
                resolver->addUse(ident, resolver->kindForSymbol(s));
                return switchTo(s->type(resolver));
            }
        }
    }

    return unresolve();
}

ExprType &ExprType::rangeValue(ResolveContext *resolver)
{
    if (size() == 1 && first().first == 0)
        return switchTo(first().second->elementsType(resolver));

    return unresolve();
}

ExprType &ExprType::keyValue(ResolveContext *resolver)
{
    if (size() == 1 && first().first == 0)
        return switchTo(first().second->indexType(resolver));

    return unresolve();
}

ExprType &ExprType::chanValue(ResolveContext *)
{
    if (size() == 1 && first().first == 0)
        return switchTo(first().second->chanValueType());

    return unresolve();
}

}   // namespace GoTools
