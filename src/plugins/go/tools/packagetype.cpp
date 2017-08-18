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
#include "packagetype.h"
#include "ast.h"
#include "scope.h"
#include "resolvedtype.h"

#include <texteditor/codeassist/assistproposalitem.h>

namespace GoTools {

PackageType::PackageType(QHash<QString, GoSource::Ptr> &sources)
    : m_sources(sources)
{ }

PackageType::~PackageType()
{ }

Symbol *PackageType::lookupMethod(const Identifier *typeId, const Identifier *funcId, ResolveContext *)
{
    for (QHash<QString, GoSource::Ptr>::const_iterator it = m_sources.constBegin();
         it != m_sources.constEnd(); ++it) {
        if (FileAST *fileAst = it.value()->translationUnit()->fileAst()) {
            if (FileScope *fileScope = fileAst->scope) {
                if (Symbol *symbol = fileScope->lookupMethod(typeId, funcId))
                    return symbol;
            }
        }
    }

    return 0;
}

void PackageType::fillMethods(QList<TextEditor::AssistProposalItemInterface *> &completions, const Identifier *typeId, ResolveContext *)
{
    for (QHash<QString, GoSource::Ptr>::const_iterator it = m_sources.constBegin();
         it != m_sources.constEnd(); ++it) {
        if (FileAST *fileAst = it.value()->translationUnit()->fileAst()) {
            if (FileScope *fileScope = fileAst->scope) {
                fileScope->fillMethods(completions, typeId);;
            }
        }
    }
}

Symbol *PackageType::lookup(const Identifier *ident) const
{
    for (QHash<QString, GoSource::Ptr>::const_iterator it = m_sources.constBegin();
         it != m_sources.constEnd(); ++it) {
        if (FileAST *fileAst = it.value()->translationUnit()->fileAst()) {
            if (FileScope *fileScope = fileAst->scope) {
                if (Symbol *symbol = fileScope->find(ident))
                    return symbol;
            }
        }
    }

    return 0;
}

Symbol *PackageType::lookupMember(const IdentAST *ident, ResolveContext *, int refLevel) const
{
    if (!refLevel)
        for (QHash<QString, GoSource::Ptr>::const_iterator it = m_sources.constBegin();
             it != m_sources.constEnd(); ++it)
            if (FileAST *fileAst = it.value()->translationUnit()->fileAst())
                if (FileScope *fileScope = fileAst->scope)
                    if (Symbol *symbol = fileScope->find(ident->ident))
                        return symbol;

    return 0;
}

void PackageType::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                        ResolveContext *, int refLevel, LookupContext::Predicate) const
{
    if (!refLevel)
        for (QHash<QString, GoSource::Ptr>::const_iterator it = m_sources.constBegin();
             it != m_sources.constEnd(); ++it)
            if (FileAST *fileAst = it.value()->translationUnit()->fileAst())
                if (FileScope *fileScope = fileAst->scope)
                    for (unsigned i = 0; i < fileScope->memberCount(); i++) {
                        Symbol *symbol = fileScope->memberAt(i);
                        TextEditor::AssistProposalItem *item = new TextEditor::AssistProposalItem;;
                        item->setText(symbol->identifier()->toString());
                        item->setIcon(Symbol::icon(symbol->kind()));
                        completions.append(item);
                    }
}

ResolvedType PackageType::indexType(ResolveContext *, int) const
{ return 0; }

ResolvedType PackageType::elementsType(ResolveContext *, int) const
{ return 0; }

ResolvedType PackageType::chanValueType(ResolveContext *, int) const
{ return 0; }

}   // namespace GoTools
