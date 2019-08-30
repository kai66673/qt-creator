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
#include "types.h"
#include "resolvedtype.h"
#include "ast.h"

#include <texteditor/codeassist/assistproposalitem.h>

namespace GoTools {

ResolvedType Type::callType(ResolveContext *, int) const
{ return Control::unresolvedTupleType(); }

ResolvedType Type::extractFromTuple(int index, int refLvl) const
{ return index ? ResolvedType() : ResolvedType(this).setRefLevel(refLvl); }

ResolvedType BuiltinType::indexType(ResolveContext *, int) const
{ return ResolvedType(); }

ResolvedType BuiltinType::elementsType(ResolveContext *, int) const
{ return ResolvedType(); }

ResolvedType BuiltinType::chanValueType(ResolveContext *, int) const
{ return ResolvedType(); }

Symbol *ErrorType::lookupMember(const IdentAST *ident, ResolveContext *, int refLvl) const
{
    if ((refLvl == 0 || refLvl == -1) && ident->ident->toString() == QLatin1String("Error"))
        return Control::errorErrorMethod();
    return 0;
}

void ErrorType::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions, ResolveContext *, int refLvl, LookupContext::Predicate) const
{
    if (refLvl == 0 || refLvl == -1) {
        TextEditor::AssistProposalItem *item = new TextEditor::AssistProposalItem;
        item->setText(QLatin1String("Error"));
        item->setIcon(Symbol::icon(Symbol::Mtd));
        completions.append(item);
    }
}

}   // namespace GoTools
