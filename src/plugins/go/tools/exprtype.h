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
#include "astfwd.h"

#include <QPair>

namespace GoTools {

class GoCheckSymbols;

typedef QPair<int, const Type *> ExprTypeItem;

class ExprType: public QList<ExprTypeItem>
{
public:
    ExprType(const Type *typ = 0, int derefLevel = 0);
    ExprType(const QList<const Type *> &types);

    const Type *type() const;
    const Type *type(int indexInTuple) const;
    const Type *typeForMemberAccess() const;
    const Type *typeForDirectAccess() const;

    Symbol *lookupMember(const IdentAST *ident, ResolveContext *resolver);
    void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ResolveContext *resolver);

    bool applyCommaJoin(ExprAST *x, ResolveContext *resolver);

    ExprType &unresolve();
    ExprType &applyIntegralOperation(ExprAST *x, ResolveContext *resolver);
    ExprType &applyPlusOperation(ExprAST *x, ResolveContext *resolver);
    ExprType &unstar();
    ExprType &deref();
    ExprType &switchTo(const Type *typ);
    ExprType &memberAccess(IdentAST *ident, ResolveContext *resolver);
    ExprType &checkMemberAccess(IdentAST *ident, GoCheckSymbols *resolver);
    ExprType &rangeValue(ResolveContext *resolver);
    ExprType &keyValue(ResolveContext *resolver);
    ExprType &chanValue(ResolveContext *);
};

}   // namespace GoTools
