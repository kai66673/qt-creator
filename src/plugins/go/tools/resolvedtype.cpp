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
#include "resolvedtype.h"
#include "control.h"
#include "ast.h"
#include "gochecksymbols.h"

namespace GoTools {

ResolvedType::ResolvedType()
    : refLevel(0)
    , typ(Control::unresolvedType())
{ }

ResolvedType::ResolvedType(const Type *t)
    : refLevel(0)
    , typ(t ? t: Control::unresolvedType())
{ }

const Type *ResolvedType::typeForMemberAccess() const
{ return refLevel == 0 || refLevel == -1 ? typ : 0; }

const Type *ResolvedType::typeForDirectAccess() const
{ return refLevel == 0 ? typ : 0; }

Symbol *ResolvedType::lookupMember(const IdentAST *ident, ResolveContext *resolver, int refLvl)
{
    if (ident && ident->isLookable())
        return typ->lookupMember(ident, resolver, refLvl + refLevel);

    return 0;
}

void ResolvedType::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                         ResolveContext *resolver,
                                         int refLvl)
{
    typ->fillMemberCompletions(completions, resolver, refLvl + refLevel);
}

QString ResolvedType::describe() const
{
    QString result;
    if (refLevel < 0)
        for (int i = refLevel; i < 0; i++)
            result += QLatin1Char('*');
    else if (refLevel > 0)
        result += QLatin1Char('?');
    return result + typ->describe();
}

ResolvedType &ResolvedType::setRefLevel(int refLvl)
{
    refLevel = refLvl;
    return *this;
}

ResolvedType &ResolvedType::unresolve()
{
    refLevel = 0;
    typ = Control::unresolvedType();
    return *this;
}

ResolvedType &ResolvedType::switchTo(const Type *t)
{
    refLevel =  0;
    typ = t ? t : Control::unresolvedType();
    return *this;
}

ResolvedType &ResolvedType::switchTo(const ResolvedType &t)
{
    refLevel = t.refLevel;
    typ = t.typ;
    return *this;
}

ResolvedType &ResolvedType::applyIntegralOperation(ExprAST *rhx, ResolveContext *resolver)
{
    if (rhx && typ->builtinKind(resolver, refLevel) == Type::Integral) {
        ResolvedType xTyp = rhx->resolve(resolver);
        if (xTyp.typ->builtinKind(resolver, xTyp.refLevel) == Type::Integral)
            return *this;
    }

    return unresolve();
}

ResolvedType &ResolvedType::applyPlusOperation(ExprAST *rhx, ResolveContext *resolver)
{
    if (rhx) {
        Type::BuiltingKind kind = typ->builtinKind(resolver, refLevel);
        if (kind == Type::String || kind == Type::Integral) {
            ResolvedType xTyp = rhx->resolve(resolver);
            if (xTyp.typ->builtinKind(resolver, xTyp.refLevel) == kind)
                return *this;
        }
    }

    return unresolve();
}

ResolvedType &ResolvedType::memberAccess(IdentAST *ident, ResolveContext *resolver)
{
    if (Symbol *s = typ->lookupMember(ident, resolver, refLevel))
        return switchTo(s->type(resolver));

    return unresolve();
}

ResolvedType &ResolvedType::checkMemberAccess(IdentAST *ident, GoCheckSymbols *resolver)
{
    if (Symbol *s = typ->lookupMember(ident, resolver, refLevel)) {
        resolver->addUse(ident, resolver->kindForSymbol(s));
        return switchTo(s->type(resolver));
    }

    return unresolve();
}

ResolvedType &ResolvedType::call(ResolveContext *resolver, int refLvl)
{ return switchTo(typ->callType(resolver, refLevel + refLvl)); }

ResolvedType &ResolvedType::rangeValue(ResolveContext *resolver, int refLvl)
{ return switchTo(typ->elementsType(resolver, refLvl + refLevel)); }

ResolvedType &ResolvedType::rangeKey(ResolveContext *resolver, int refLvl)
{ return switchTo(typ->indexType(resolver, refLvl + refLevel)); }

ResolvedType &ResolvedType::chanValue(ResolveContext *resolver, int refLvl)
{ return switchTo(typ->chanValueType(resolver, refLvl + refLevel)); }

ResolvedType &ResolvedType::unstar()
{ ++refLevel; return *this; }

ResolvedType &ResolvedType::deref()
{ --refLevel; return *this; }

}   // namespace GoTools
