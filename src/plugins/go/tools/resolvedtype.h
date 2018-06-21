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

#include "types.h"

namespace GoTools {

class GoCheckSymbols;

class ResolvedType
{
    int m_referenceLevel;
    const Type *m_type;

public:
    ResolvedType();
    ResolvedType(const Type *t);

    const Type *type() const { return m_type; }
    int referenceLevel() const { return m_referenceLevel; }

    const Type *typeForMemberAccess() const;
    const Type *typeForDirectAccess() const;
    Symbol *lookupMember(const IdentAST *ident, ResolveContext *resolver, int refLvl = 0);
    void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                               ResolveContext *resolver,
                               int refLvl = 0);

    QString describe() const;

    ResolvedType &setRefLevel(int refLvl);

    ResolvedType &unresolve();
    ResolvedType &switchTo(const Type *t);
    ResolvedType &switchTo(const ResolvedType &t);

    ResolvedType &applyIntegralOperation(ExprAST *rhx, ResolveContext *resolver);
    ResolvedType &applyPlusOperation(ExprAST *rhx, ResolveContext *resolver);
    ResolvedType &memberAccess(IdentAST *ident, ResolveContext *resolver);
    ResolvedType &checkMemberAccess(IdentAST *ident, GoTools::GoCheckSymbols *resolver);
    ResolvedType &call(ResolveContext *resolver, int refLvl = 0);
    ResolvedType &rangeValue(ResolveContext *resolver, int refLvl = 0);
    ResolvedType &rangeKey(ResolveContext *resolver, int refLvl = 0);
    ResolvedType &rangeKeyOrChanType(ResolveContext *resolver, int refLvl = 0);
    ResolvedType &chanValue(ResolveContext *resolver, int refLvl = 0);
    ResolvedType &unstar();
    ResolvedType &deref();
};

}   // namespace GoTools
