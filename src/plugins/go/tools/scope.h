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

#include "symbol.h"
#include "symboltable.h"
#include "methodsscope.h"
#include "lookupcontext.h"

#include <texteditor/texteditor.h>

namespace GoTools {

class PackageType;
class SymbolTable;

class Scope: public LookupContext
{
public:
    Scope(Scope *outer);
    virtual ~Scope();

    /// Adds a Symbol to this Scope.
    void addMember(Symbol *symbol);

    /// Returns true if this Scope is empty; otherwise returns false.
    bool isEmpty() const;

    /// Returns the number of symbols is in the scope.
    unsigned memberCount() const;

    /// Returns the Symbol at the given position.
    Symbol *memberAt(unsigned index) const;

    typedef Symbol **iterator;

    /// Returns member iterator to the beginning.
    iterator memberBegin() const;

    /// Returns member iterator to the end.
    iterator memberEnd() const;

    Symbol *find(const HashedLiteral *identifier) const;

    Scope *outer() const;

    // LookupContext implementation
    virtual Symbol *lookupMember(const IdentAST *ident,
                                 ResolveContext *resolver,
                                 int refLevel = 0) const override;

    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ResolveContext *resolver,
                                       int refLevel = 0,
                                       Predicate predicate = 0) const override;

    virtual FileScope *asFileScope() { return 0; }
    virtual bool isLocal() const { return m_outer; }

    void dump() const;
    FileScope *fileScope();

    AST *ast() const;
    virtual AST *enclosingAst() const;
    void setAst(AST *ast);

protected:
    Scope *m_outer;
    SymbolTable *_members;
    AST *m_ast;
};

class StructScope: public Scope
{
public:
    StructScope(Scope *outer)
        : Scope(outer)
    { }

    virtual AST *enclosingAst() const override
    { return m_outer ? m_outer->enclosingAst() : 0; }

    virtual bool isLocal() const override
    { return m_outer ? m_outer->isLocal() : false; }
};

class FileScope: public Scope
{
public:
    FileScope(GoSource *source);
    void declareMethod(const Identifier *typeId, FuncDeclAST *funcDecl);

    // LookupContext implementation
    virtual Symbol *lookupMember(const IdentAST *ident,
                                 ResolveContext *resolver,
                                 int refLevel = 0) const override;

    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ResolveContext *resolver,
                                       int refLevel = 0,
                                       Predicate predicate = 0) const override;

    Symbol *lookupMethod(const Identifier *typeId, const Identifier *funcId);
    void fillMethods(QList<TextEditor::AssistProposalItemInterface *> &completions,
                     const Identifier *typeId);

    virtual FileScope *asFileScope() override { return this; }

    void fillLink(Utils::Link &link, unsigned tokenIndex);

    GoSource *source() const;

    QHash<QString, PackageType *> &aliasToLookupContext();

private:
    GoSource *m_source;
    MethodsScope m_methods;

    QHash<QString, PackageType *> m_aliasToLookupContext;
};

}   // namespace GoTools
