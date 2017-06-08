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
#include "gosnapshot.h"
#include "ast.h"
#include "scope.h"
#include "exprtyperesolver.h"

#include <texteditor/codeassist/assistproposalitem.h>

namespace GoTools {

PackageType::PackageType()
{ }

PackageType::~PackageType()
{ }

Symbol *PackageType::lookupMethod(const Identifier *typeId, const Identifier *funcId)
{
    for (FileScope *scope: m_fileScopes)
        if (Symbol *symbol = scope->lookupMethod(typeId, funcId))
            return symbol;

    return 0;
}

void PackageType::fillMethods(QList<TextEditor::AssistProposalItemInterface *> &completions,
                              const Identifier *typeId)
{
    for (FileScope *scope: m_fileScopes)
        scope->fillMethods(completions, typeId);
}

Symbol *PackageType::lookupMember(const IdentAST *ident, ExprTypeResolver *) const
{
    for (FileScope *scope: m_fileScopes)
        if (Symbol *symbol = scope->find(ident->ident))
            return symbol;

    return 0;
}

void PackageType::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                        ExprTypeResolver *, Predicate) const
{
    for (FileScope *scope: m_fileScopes) {
        for (unsigned i = 0; i < scope->memberCount(); i++) {
            Symbol *symbol = scope->memberAt(i);
            TextEditor::AssistProposalItem *item = new TextEditor::AssistProposalItem;;
            item->setText(symbol->identifier()->toString());
            item->setIcon(Symbol::icon(symbol->kind()));
            completions.append(item);
        }
    }
}

const Type *PackageType::indexType(ExprTypeResolver *) const
{ return 0; }

const Type *PackageType::elementsType(ExprTypeResolver *) const
{ return 0; }

const Type *PackageType::calleeType(int, ExprTypeResolver *) const
{ return 0; }

const Type *PackageType::chanValueType() const
{ return 0; }

GoSnapshot::GoSnapshot(const QHash<GoPackageKey, GoPackage *> &packages)
{
    for (QHash<GoPackageKey, GoPackage *>::const_iterator it = packages.constBegin();
         it != packages.constEnd(); ++it) {
        PackageType *lookupContext = new PackageType();
        m_packages.insert(it.key(), lookupContext);
        GoPackage *package = it.value();
        for (const GoSource::Ptr &doc: package->sources().values()) {
            if (FileAST *fileAst = doc->translationUnit()->fileAst()) {
                lookupContext->m_fileScopes.append(fileAst->scope);
                fileAst->scope->setPackageType(lookupContext);
            }
        }
    }
}

GoSnapshot::~GoSnapshot()
{
    qDeleteAll(m_packages.values());
}

static QMutex s_mutex;

void GoSnapshot::runProtectedTask(GoSnapshot::ProtectedTask task)
{
    QMutexLocker lock(&s_mutex);
    task();
}

PackageType *GoSnapshot::packageTypeForImport(const GoSource::Import &import)
{
    auto context_it = m_packages.constFind({import.resolvedDir, import.packageName});
    if (context_it != m_packages.constEnd()) {
        return context_it.value();
    }
    return 0;
}

}   // namespace GoTools
