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
#include "resolvecontext.h"
#include "gopackage.h"
#include "packagetype.h"
#include "ast.h"
#include "gocodemodelmanager.h"
#include "gochecksymbols.h"

namespace GoTools {

ResolveContext::ResolveContext(GoSource *source, bool protectCache)
    : m_protectCache(protectCache)
    , m_cache(GoPackageCache::instance())
    , m_initialFileAst(0)
    , m_currentScope(0)
    , m_initialFileScope(0)
    , m_isValid(false)
    , m_workingCopyIsObtained(false)
{
    if (m_protectCache)
        GoPackageCache::instance()->acquireCache();

    if (source) {
        if (FileAST *fileAst = source->translationUnit()->fileAst()) {
            m_initialFileAst = fileAst;
            m_currentScope = fileAst->scope;
            m_initialFileScope = fileAst->scope;
            m_isValid = true;
        }
    }
}

ResolveContext::~ResolveContext()
{
    if (m_protectCache)
        GoPackageCache::instance()->releaseCache();
}

Scope *ResolveContext::switchScope(Scope *scope)
{
    if (! scope)
        return m_currentScope;

    std::swap(m_currentScope, scope);
    return scope;
}

Scope *ResolveContext::currentScope() const
{ return m_currentScope; }

bool ResolveContext::isValidResolveContext() const
{ return m_isValid; }

PackageType *ResolveContext::packageTypeForAlias(const QString &packageAlias, FileScope *fileScope)
{
    if (!fileScope)
        fileScope = m_initialFileScope;
    if (!fileScope)
        return 0;

    QHash<QString, PackageType *> &aliasToLookupContext = fileScope->aliasToLookupContext();
    auto it = aliasToLookupContext.constFind(packageAlias);
    if (it != aliasToLookupContext.constEnd())
        return it.value();

    for (const GoSource::Import &import: fileScope->source()->imports()) {
        if (import.alias == QStringLiteral("_"))
            continue;
        if (import.alias == packageAlias) {
            auto context_it = m_cache->m_packages.constFind({import.resolvedDir, import.packageName});
            if (context_it != m_cache->m_packages.constEnd()) {
                PackageType *pkgType = context_it.value()->type();
                aliasToLookupContext.insert(packageAlias, pkgType);
                return pkgType;
            }
            if (!m_workingCopyIsObtained) {
                m_workingCopyIsObtained = true;
                m_workingCopy = GoCodeModelManager::instance()->buildWorkingCopy();
            }
            PackageType *newPackageType = m_cache->importPackage({import.resolvedDir, import.packageName}, m_workingCopy)->type();
            aliasToLookupContext.insert(packageAlias, newPackageType);
            return newPackageType;
        }
    }

    return 0;
}

PackageType *ResolveContext::fileScopePackageType(const FileScope *fileScope) const
{ return fileScope->source()->package()->type(); }

QList<GoSource *> ResolveContext::sources() const
{
    QList<GoSource *> result;
    for (GoPackage *pkg: GoPackageCache::instance()->m_packages)
        for (const GoSource::Ptr &doc: pkg->m_sources)
            result.push_back(doc.data());

    return result;
}

const Type *tryResolveNamedType(ResolveContext *resolver, ExprAST *x)
{
    if (x) {
        if (IdentAST *ident = x->asIdent()) {
            if (ident->isLookable()) {
                if (Symbol *s = resolver->currentScope()->lookupMember(ident, resolver)) {
                    if (s->kind() == Symbol::Typ) {
                        return s->type(resolver).type();   // TODO:
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
                                return s->type(resolver).type();   // TODO:
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

const Type *tryCheckNamedType(GoCheckSymbols *resolver, ExprAST *x) // TODO:
{
    if (x) {
        if (IdentAST *ident = x->asIdent()) {
            if (ident->isLookable()) {
                if (Symbol *s = resolver->currentScope()->lookupMember(ident, resolver)) {
                    if (s->kind() == Symbol::Typ) {
                        resolver->addUse(ident, GoSemanticHighlighter::Type);
                        return s->type(resolver).type();
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
                                resolver->addUse(packageIdent, GoSemanticHighlighter::Package);
                                resolver->addUse(selExpr->sel, GoSemanticHighlighter::Type);
                                return s->type(resolver).type();
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
