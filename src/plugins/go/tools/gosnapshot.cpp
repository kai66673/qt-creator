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

Symbol *PackageType::lookupMethod(const Identifier *typeId, const Identifier *funcId, GoSnapshot *snapshot)
{
    for (int i = m_first; i < m_last; i++) {
        FileScope *scope = snapshot->m_scopeTable.at(i)->scope;
        if (Symbol *symbol = scope->lookupMethod(typeId, funcId))
            return symbol;
    }

    return 0;
}

void PackageType::fillMethods(QList<TextEditor::AssistProposalItemInterface *> &completions,
                              const Identifier *typeId, GoSnapshot *snapshot)
{
    for (int i = m_first; i < m_last; i++) {
        FileScope *scope = snapshot->m_scopeTable.at(i)->scope;
        scope->fillMethods(completions, typeId);
    }
}

Symbol *PackageType::lookupMember(const IdentAST *ident, ExprTypeResolver *resolver)
{
    for (int i = m_first; i < m_last; i++) {
        Scope *scope = resolver->snapshot()->m_scopeTable.at(i)->scope;
        if (Symbol *symbol = scope->find(ident->ident))
            return symbol;
    }

    return 0;
}

void PackageType::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                        ExprTypeResolver *resolver, Predicate)
{
    for (int i = m_first; i < m_last; i++) {
        Scope *scope = resolver->snapshot()->m_scopeTable.at(i)->scope;
        for (unsigned i = 0; i < scope->memberCount(); i++) {
            Symbol *symbol = scope->memberAt(i);
            TextEditor::AssistProposalItem *item = new TextEditor::AssistProposalItem;;
            item->setText(symbol->identifier()->toLatin1());
            item->setIcon(Symbol::icon(symbol->kind()));
            completions.append(item);
        }
    }
}

Type *PackageType::indexType(ExprTypeResolver *)
{ return 0; }

Type *PackageType::elementsType(ExprTypeResolver *)
{ return 0; }

Type *PackageType::calleeType(int, ExprTypeResolver *) const
{ return 0; }

Type *PackageType::chanValueType() const
{ return 0; }

GoSnapshot::GoSnapshot(const QHash<GoPackageKey, GoPackage *> &packages)
{
    int index = 0;
    for (QHash<GoPackageKey, GoPackage *>::const_iterator it = packages.constBegin();
         it != packages.constEnd(); ++it) {
        PackageType *lookupContext = new PackageType();
        lookupContext->m_first = index;
        m_packages.insert(it.key(), lookupContext);
        GoPackage *package = it.value();
        for (const GoSource::Ptr &doc: package->sources().values()) {
            ScopeRecord *record = new ScopeRecord();
            record->source = doc;
            if (FileAST *fileAst = doc->translationUnit()->fileAst()) {
                record->scope = fileAst->scope;
                record->lookupcontext = lookupContext;
                m_scopeTable.append(record);
                index++;
            }
        }
        lookupContext->m_last = index;
    }

    bindScopes();
}

GoSnapshot::~GoSnapshot()
{
    qDeleteAll(m_packages.values());
    qDeleteAll(m_scopeTable);
}

static QMutex s_mutex;

void GoSnapshot::runProtectedTask(GoSnapshot::ProtectedTask task)
{
    QMutexLocker lock(&s_mutex);
    task();
}

void GoSnapshot::bindScopes()
{
    int i = 0;
    for (ScopeRecord *record: m_scopeTable) {
        record->scope->setIndexInSnapshot(i);
        i++;
    }
}

PackageType *GoSnapshot::packageTypetAt(int index) const
{ return m_scopeTable.at(index)->lookupcontext; }

FileScope *GoSnapshot::fileScopeAt(int index) const
{ return m_scopeTable.at(index)->scope; }

PackageType *GoSnapshot::packageTypeForAlias(int index, const QLatin1String &alias)
{
    ScopeRecord *record = m_scopeTable.at(index);

    auto it = record->aliasToLookupContext.constFind(alias);
    if (record->aliasToLookupContext.constFind(alias) != record->aliasToLookupContext.constEnd())
        return it.value();

    for (const GoSource::Import &import: record->source->imports()) {
        if (import.alias == QStringLiteral("_"))
            continue;
        if (import.alias == alias) {
            auto context_it = m_packages.constFind({import.resolvedDir, import.packageName});
            if (context_it != m_packages.constEnd()) {
                record->aliasToLookupContext.insert(alias, context_it.value());
                return context_it.value();
            }
            return 0;
        }
    }
    return 0;
}

}   // namespace GoTools
