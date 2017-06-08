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
#include "scope.h"
#include "ast.h"
#include "gosnapshot.h"
#include "exprtyperesolver.h"

#include <texteditor/codeassist/assistproposalitem.h>

#include <QDebug>
#include <QString>

namespace GoTools {

Scope::Scope(Scope *outer)
    : m_outer(outer)
    , _members(new SymbolTable)
{ }

Scope::~Scope()
{ delete _members; }

void Scope::addMember(Symbol *symbol)
{ _members->enterSymbol(symbol); }

bool Scope::isEmpty() const
{ return _members->isEmpty(); }

unsigned Scope::memberCount() const
{ return _members->symbolCount(); }

Symbol *Scope::memberAt(unsigned index) const
{ return _members->symbolAt(index); }

Scope::iterator Scope::memberBegin() const
{ return _members->firstSymbol(); }

Scope::iterator Scope::memberEnd() const
{ return _members->lastSymbol(); }

Symbol *Scope::find(const HashedLiteral *name) const
{ return _members->lookat(name); }

Scope *Scope::outer() const
{ return m_outer; }

Symbol *Scope::lookupMember(const IdentAST *ident, ExprTypeResolver *resolver) const
{
    if (Symbol *s = find(ident->ident)) {
        unsigned lastToken = s->declExpr() ? s->declExpr()->lastToken() : s->sourceLocation();
        if (lastToken < ident->t_identifier)
            return s;
    }
    return m_outer ? m_outer->lookupMember(ident, resolver) : 0;
}

void Scope::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                  ExprTypeResolver *resolver, Predicate predicate) const
{
    for (unsigned i = 0; i < memberCount(); i++) {
        Symbol *symbol = memberAt(i);
        if (!predicate || predicate(symbol)) {
            TextEditor::AssistProposalItem *item = new TextEditor::AssistProposalItem;;
            item->setText(symbol->identifier()->toString());
            item->setIcon(Symbol::icon(symbol->kind()));
            completions.append(item);
        }
    }

    if (m_outer)
        m_outer->fillMemberCompletions(completions, resolver);
}

void Scope::dump() const
{
    for (unsigned i = 0; i < memberCount(); i++) {
        Symbol *s = memberAt(i);
        qDebug() << " ===" << s->identifier()->toString() << "kind:" << s->kind();
    }
}

FileScope *Scope::fileScope()
{ return m_outer ? m_outer->fileScope() : asFileScope(); }

FileScope::FileScope(GoSource *source)
    : Scope(0)
    , m_source(source)
    , m_packageType(0)
{ }

void FileScope::declareMethod(const Identifier *typeId, FuncDeclAST *funcDecl)
{ m_methods.addMethod(typeId, funcDecl); }

Symbol *FileScope::lookupMember(const IdentAST *ident, ExprTypeResolver *resolver) const
{
    return m_packageType
            ? m_packageType->lookupMember(ident, resolver)
            : find(ident->ident);
}

void FileScope::fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                      ExprTypeResolver *resolver, Predicate predicate) const
{
    if (!m_packageType) {
        for (unsigned i = 0; i < memberCount(); i++) {
            Symbol *symbol = memberAt(i);
            TextEditor::AssistProposalItem *item = new TextEditor::AssistProposalItem;;
            item->setText(symbol->identifier()->toString());
            item->setIcon(Symbol::icon(symbol->kind()));
            completions.append(item);
        }
        return;
    }

    m_packageType->fillMemberCompletions(completions, resolver, predicate);
}

Symbol *FileScope::lookupMethod(const Identifier *typeId, const Identifier *funcId)
{
    if (FuncDeclAST *funcDecl = m_methods.lookup(typeId, funcId))
        return funcDecl->symbol;
    return 0;
}

void FileScope::fillMethods(QList<TextEditor::AssistProposalItemInterface *> &completions,
                            const Identifier *typeId)
{
    for (const QString &method: m_methods.typeMethods(typeId)) {
        TextEditor::AssistProposalItem *item = new TextEditor::AssistProposalItem;;
        item->setText(method);
        item->setIcon(Symbol::icon(Symbol::Fun));
        completions.append(item);
    }
}

void FileScope::fillLink(TextEditor::TextEditorWidget::Link &link, unsigned tokenIndex)
{
    const Token &tk = m_source->translationUnit()->tokenAt(tokenIndex);
    link.targetFileName = m_source->fileName();
    link.targetLine = tk.kindAndPos.line;
    link.targetColumn = tk.kindAndPos.column - 1;
}

GoSource *FileScope::source() const
{ return m_source; }

PackageType *FileScope::packageType() const
{ return m_packageType; }

void FileScope::setPackageType(PackageType *packageType)
{ m_packageType = packageType; }

PackageType *FileScope::packageTypeForAlias(const QString &alias, GoSnapshot *snapshot)
{
    auto it = m_aliasToLookupContext.constFind(alias);
    if (m_aliasToLookupContext.constFind(alias) != m_aliasToLookupContext.constEnd())
        return it.value();

    for (const GoSource::Import &import: m_source->imports()) {
        if (import.alias == QStringLiteral("_"))
            continue;
        if (import.alias == alias) {
            PackageType *pkg = snapshot->packageTypeForImport(import);
            if (pkg)
                m_aliasToLookupContext.insert(alias, pkg);
            return pkg;
        }
    }
    return 0;
}

}   // namespace GoTools
