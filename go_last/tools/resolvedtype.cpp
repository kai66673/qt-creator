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
    : m_referenceLevel(0)
    , m_type(Control::unresolvedType())
{ }

ResolvedType::ResolvedType(const Type *t)
    : m_referenceLevel(0)
    , m_type(t ? t: Control::unresolvedType())
{ }

const Type *ResolvedType::typeForMemberAccess() const
{ return m_referenceLevel == 0 || m_referenceLevel == -1 ? m_type : 0; }

const Type *ResolvedType::typeForDirectAccess() const
{ return m_referenceLevel == 0 ? m_type : 0; }

Symbol *ResolvedType::lookupMember(const IdentAST *ident, ResolveContext *resolver, int refLvl)
{
    if (ident && ident->isLookable())
        return m_type->lookupMember(ident, resolver, refLvl + m_referenceLevel);

    return 0;
}

void ResolvedType::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                         ResolveContext *resolver,
                                         int refLvl)
{
    m_type->fillMemberCompletions(completions, resolver, refLvl + m_referenceLevel);
}

QString ResolvedType::describe() const
{
    QString result;
    if (m_referenceLevel < 0)
        for (int i = m_referenceLevel; i < 0; i++)
            result += QLatin1Char('*');
    else if (m_referenceLevel > 0)
        result += QLatin1Char('?');
    return result + m_type->describe();
}

ResolvedType &ResolvedType::setRefLevel(int refLvl)
{
    m_referenceLevel = refLvl;
    return *this;
}

ResolvedType &ResolvedType::unresolve()
{
    m_referenceLevel = 0;
    m_type = Control::unresolvedType();
    return *this;
}

ResolvedType &ResolvedType::switchTo(const Type *t)
{
    m_referenceLevel =  0;
    m_type = t ? t : Control::unresolvedType();
    return *this;
}

ResolvedType &ResolvedType::switchTo(const ResolvedType &t)
{
    m_referenceLevel = t.m_referenceLevel;
    m_type = t.m_type;
    return *this;
}

ResolvedType &ResolvedType::applyIntegralOperation(ExprAST *rhx, ResolveContext *resolver)
{
    if (rhx && m_type->builtinKind(resolver, m_referenceLevel) == Type::Integral) {
        ResolvedType xTyp = rhx->resolve(resolver);
        if (xTyp.m_type->builtinKind(resolver, xTyp.m_referenceLevel) == Type::Integral)
            return *this;
    }

    return unresolve();
}

ResolvedType &ResolvedType::applyPlusOperation(ExprAST *rhx, ResolveContext *resolver)
{
    if (rhx) {
        Type::BuiltingKind kind = m_type->builtinKind(resolver, m_referenceLevel);
        if (kind == Type::String || kind == Type::Integral) {
            ResolvedType xTyp = rhx->resolve(resolver);
            if (xTyp.m_type->builtinKind(resolver, xTyp.m_referenceLevel) == kind)
                return *this;
        }
    }

    return unresolve();
}

ResolvedType &ResolvedType::memberAccess(IdentAST *ident, ResolveContext *resolver)
{
    if (Symbol *s = m_type->lookupMember(ident, resolver, m_referenceLevel))
        return switchTo(s->type(resolver));

    return unresolve();
}

ResolvedType &ResolvedType::checkMemberAccess(IdentAST *ident, GoCheckSymbols *resolver)
{
    if (Symbol *s = m_type->lookupMember(ident, resolver, m_referenceLevel)) {
        resolver->addUse(ident, resolver->kindForSymbol(s));
        return switchTo(s->type(resolver));
    }

    return unresolve();
}

ResolvedType &ResolvedType::call(ResolveContext *resolver, int refLvl)
{ return switchTo(m_type->callType(resolver, m_referenceLevel + refLvl)); }

ResolvedType &ResolvedType::rangeValue(ResolveContext *resolver, int refLvl)
{ return switchTo(m_type->elementsType(resolver, refLvl + m_referenceLevel)); }

ResolvedType &ResolvedType::rangeKey(ResolveContext *resolver, int refLvl)
{ return switchTo(m_type->indexType(resolver, refLvl + m_referenceLevel)); }

ResolvedType &ResolvedType::rangeKeyOrChanType(ResolveContext *resolver, int refLvl)
{
    auto chanType = m_type->chanValueType(resolver, refLvl + m_referenceLevel);
    if (chanType.m_type != Control::unresolvedType())
        return switchTo(chanType);
    return switchTo(m_type->indexType(resolver, refLvl + m_referenceLevel));
}

ResolvedType &ResolvedType::chanValue(ResolveContext *resolver, int refLvl)
{ return switchTo(m_type->chanValueType(resolver, refLvl + m_referenceLevel)); }

ResolvedType &ResolvedType::unstar()
{ ++m_referenceLevel; return *this; }

ResolvedType &ResolvedType::deref()
{ --m_referenceLevel; return *this; }

}   // namespace GoTools
