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
#include "gochecksymbols.h"
#include "ast.h"
#include "control.h"
#include "packagetype.h"

#include <utils/algorithm.h>
#include <utils/qtcassert.h>

#include <QDateTime>

namespace GoTools {

GoSemanticHighlighter::Kind GoCheckSymbols::kindForSymbol(const Symbol *symbol) const
{
    switch (symbol->kind()) {
        case Symbol::Pkg: return GoSemanticHighlighter::Package;
        case Symbol::Con: return GoSemanticHighlighter::Const;
        case Symbol::Typ: return GoSemanticHighlighter::Type;
        case Symbol::Var: return GoSemanticHighlighter::Var;
        case Symbol::Fld: return GoSemanticHighlighter::Field;
        case Symbol::Fun: return GoSemanticHighlighter::Func;
        case Symbol::Mtd: return GoSemanticHighlighter::Func;
        case Symbol::Lbl: return GoSemanticHighlighter::Label;
        default: break; // prevent -Wswitch warning
    }

    return GoSemanticHighlighter::Var;
}

GoCheckSymbols::GoCheckSymbols(GoSource *source)
    : ScopeSwitchVisitor(source)
    , m_source(source)
    , m_control(source->translationUnit()->control())
    , _chunkSize(50)
{
    _tokens = translationUnit()->tokens();
    _usages.reserve(_chunkSize);
}

void GoCheckSymbols::run()
{
    if (!isCanceled() && isValidResolveContext() && !m_source->isTooBig()) {
        if (IdentAST *packageName = m_initialFileAst->packageName)
            addUseCheckFirstLine(packageName, GoSemanticHighlighter::Package);
        accept(m_initialFileAst->importDecls);
        accept(m_initialFileAst->decls);
        flush();
    }

    reportFinished();
}

static bool sortByLinePredicate(const GoCheckSymbols::Result &lhs, const GoCheckSymbols::Result &rhs)
{
    if (lhs.line == rhs.line)
        return lhs.column < rhs.column;
    else
        return lhs.line < rhs.line;
}

void GoCheckSymbols::flush()
{
    Utils::sort(_usages, sortByLinePredicate);
    reportResults(_usages);
    int cap = _usages.capacity();
    _usages.clear();
    _usages.reserve(cap);
}

void GoCheckSymbols::addUse(const IdentAST *ast, GoCheckSymbols::Kind kind)
{
    if (!ast->ident || ast->ident == Control::underscoreIdentifier())
        return;

    const Token &tok = _tokens->at(ast->t_identifier);
    _usages.append(Result(tok.line(), tok.column(), tok.length(), kind));
}

void GoCheckSymbols::addUseCheckFirstLine(IdentAST *ast, GoCheckSymbols::Kind kind)
{
    if (!ast->ident || ast->ident == Control::underscoreIdentifier())
        return;

    const Token &tok = _tokens->at(ast->t_identifier);
    unsigned line = tok.line();
    unsigned column = tok.column();
    if (line == 1)
        column++;
    _usages.append(Result(line, column, tok.length(), kind));
}

bool GoCheckSymbols::preVisit(AST *)
{ return !isCanceled(); }

bool GoCheckSymbols::visit(ImportSpecAST *ast)
{
    if (ast->name) {
        addUse(ast->name, GoSemanticHighlighter::Package);
    } else if (ast->t_path) {
        const Token &tk = _tokens->at(ast->t_path);
        QString path = tk.string->unquoted();
        if (!path.isEmpty()) {
            int pos = path.lastIndexOf('/') + 1;
            _usages.append(Result(tk.line(), tk.column() + pos + 1, path.length() - pos, GoSemanticHighlighter::Package));
        }
    }
    return false;
}

bool GoCheckSymbols::visit(FieldAST *ast)
{
    for (DeclIdentListAST *it = ast->names; it; it = it->next)
        if (DeclIdentAST *ident = it->value)
            if (ident->isLookable() && ident->symbol)
                switch (ident->symbol->kind()) {
                    case Symbol::Fld:
                        addUse(ident, GoSemanticHighlighter::Field);
                        break;
                    case Symbol::Fun:
                        addUse(ident, GoSemanticHighlighter::FuncDecl);
                        break;
                    default:
                        addUse(ident, GoSemanticHighlighter::Arg);
                        break;
                }

    accept(ast->type);
    return false;
}

bool GoCheckSymbols::visit(TypeIdentAST *ast)
{
    if (IdentAST *ident = ast->ident)
        if (ident->isLookable())
            if (Symbol *symbol = m_currentScope->lookupMember(ident, this))
                if (symbol->kind() == Symbol::Typ)
                    addUse(ident, GoSemanticHighlighter::Type);

    return false;
}

bool GoCheckSymbols::visit(PackageTypeAST *ast)
{
    QString packageAlias(ast->packageAlias->ident->toString());
    if (PackageType *context = packageTypeForAlias(packageAlias)) {
        addUse(ast->packageAlias, GoSemanticHighlighter::Package);

        if (ast->typeName->isLookable())
            if (Symbol *symbol = context->lookupMember(ast->typeName, this))
                if (symbol->kind() == Symbol::Typ)
                    addUse(ast->typeName, GoSemanticHighlighter::Type);
    }

    return false;
}

bool GoCheckSymbols::visit(SelectorExprAST *ast)
{
    if (Symbol *s = ast->x ? ast->x->check(this).lookupMember(ast->sel, this) : 0)
        addUse(ast->sel, kindForSymbol(s));

    return false;
}

const Type *GoCheckSymbols::acceptCompositLiteral(const CompositeLitAST *ast)
{
    const Type *type = 0;
    if (ExprAST *typeExpr = ast->type) {
        type = typeExpr->asType();
        if (!type) {
            type = tryCheckNamedType(this, ast->type);
        } else {
            accept(ast->type);
        }
    } else if (!m_nestedCimpositLitType.empty()) {
        type = m_nestedCimpositLitType.top();
        if (type)
            type = type->elementsType(this).type();
    }

    m_nestedCimpositLitType.push(type);
    accept(ast->elements);
    m_nestedCimpositLitType.pop();

    return type;
}

bool GoCheckSymbols::visit(CompositeLitAST *ast)
{
    acceptCompositLiteral(ast);
    return false;
}

bool GoCheckSymbols::visit(KeyValueExprAST *ast)
{
    const Type *elementsType = m_nestedCimpositLitType.empty() ? 0 : m_nestedCimpositLitType.top();
    if (elementsType && ast->key) {
        if (IdentAST *keyIdent = ast->key->asIdent()) {
            if (keyIdent->isLookable() && elementsType->lookupMember(keyIdent, this))
                addUse(keyIdent, GoSemanticHighlighter::Field);
            accept(ast->value);
            return false;
        }
    }
    return true;
}

bool GoCheckSymbols::visit(IdentAST *ast)
{
    if (ast->isLookable()) {
        if (Symbol *symbol = m_currentScope->lookupMember(ast, this))
            addUse(ast, kindForSymbol(symbol));
    }
    return false;
}

bool GoCheckSymbols::visit(DeclIdentAST *ast)
{
    if (ast->symbol) {
        switch (ast->symbol->kind()) {
            case Symbol::Typ:
                addUse(ast, GoSemanticHighlighter::TypeDecl);
                break;
            case Symbol::Con:
                addUse(ast, GoSemanticHighlighter::ConstDecl);
                break;
            case Symbol::Fun:
            case Symbol::Mtd:
                addUse(ast, GoSemanticHighlighter::FuncDecl);
                break;
            default:
                addUse(ast, GoSemanticHighlighter::VarDecl);
                break;
        }
    }

    return false;
}

}   // namespace GoTools
