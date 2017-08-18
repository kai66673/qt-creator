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
#include "ast.h"
#include "symbolundercursor.h"
#include "packagetype.h"

namespace GoTools {

SymbolUnderCursor::SymbolUnderCursor(GoSource::Ptr doc, bool protectCache)
    : ScopePositionVisitor(doc.data(), protectCache)
    , m_doc(doc)
    , m_symbol(0)
    , m_token(0)
{ }

bool SymbolUnderCursor::visit(StructTypeAST *ast)
{
    m_structures.push(ast);
    return true;
}

void SymbolUnderCursor::endVisit(StructTypeAST *)
{
    m_structures.pop();
}

bool SymbolUnderCursor::visit(DeclIdentAST *ast)
{
    const Token &tk = _tokens->at(ast->t_identifier);
    m_token = &tk;

    if (m_pos >= tk.begin() && m_pos <= tk.end()) {
        _traverseFinished = true;
        m_symbol = ast->symbol;
    }

    else if (m_pos <= tk.end())
        _traverseFinished = true;

    return false;
}

bool SymbolUnderCursor::visit(IdentAST *ast)
{
    const Token &tk = _tokens->at(ast->t_identifier);
    m_token = &tk;

    if (m_pos >= tk.begin() && m_pos <= tk.end()) {
        _traverseFinished = true;
        if (ast->isLookable())
            m_symbol = m_currentScope->lookupMember(ast, this);
    }

    else if (m_pos <= tk.end())
        _traverseFinished = true;

    return false;
}

bool SymbolUnderCursor::visit(TypeIdentAST *ast)
{
    const Token &tk = _tokens->at(ast->ident->t_identifier);
    m_token = &tk;

    if (m_pos >= tk.begin() && m_pos <= tk.end()) {
        _traverseFinished = true;
        IdentAST *ident = ast->ident;
        if (ident->isLookable())
            m_symbol = m_currentScope->lookupMember(ident, this);
    }

    else if (m_pos <= tk.end())
        _traverseFinished = true;

    return false;
}

bool SymbolUnderCursor::visit(PackageTypeAST *ast)
{
    const Token &tk = _tokens->at(ast->typeName->t_identifier);
    m_token = &tk;

    if (m_pos >= tk.begin() && m_pos <= tk.end()) {
        _traverseFinished = true;
        if (ast->typeName->isLookable()) {
            QString packageAlias(ast->packageAlias->ident->toString());
            if (PackageType *context = packageTypeForAlias(packageAlias))
                m_symbol = context->lookupMember(ast->typeName, this);
        }
    }

    else if (m_pos <= tk.end())
        _traverseFinished = true;

    return false;
}

bool SymbolUnderCursor::visit(SelectorExprAST *ast)
{
    if (!ast->sel)
        return true;

    const Token &tk = _tokens->at(ast->sel->t_identifier);
    m_token = &tk;

    if (m_pos < tk.begin()) {
        accept(ast->x);
        return false;
    }

    if (m_pos <= tk.end()) {
        _traverseFinished = true;
        if (ast->sel->isLookable())
            m_symbol = ast->x->resolve(this).lookupMember(ast->sel, this);
    }

    return false;
}

bool SymbolUnderCursor::visit(CompositeLitAST *ast)
{
    if (ast->elements) {
        const Token &firstToken = _tokens->at(ast->elements->firstToken());
        const Token &lastToken = _tokens->at(ast->elements->lastToken());
        if (m_pos >= firstToken.begin() && m_pos <= lastToken.end()) {
            const Type *type = 0;
            if (ExprAST *typeExpr = ast->type) {
                type = typeExpr->asType();
                if (!type) {
                    type = tryResolveNamedType(this, typeExpr);
                }
            } else if (!m_nestedCompositLitType.empty()) {
                type = m_nestedCompositLitType.top();
                if (type)
                    type = type->elementsType(this).type();
            }

            m_nestedCompositLitType.push(type);
            accept(ast->elements);
            m_nestedCompositLitType.pop();
            return false;
        }
    }

    return true;
}

bool SymbolUnderCursor::visit(KeyValueExprAST *ast)
{
    const Type *elementsType = m_nestedCompositLitType.empty() ? 0 : m_nestedCompositLitType.top();
    if (elementsType && ast->key) {
        if (IdentAST *keyIdent = ast->key->asIdent()) {
            const Token &tk = _tokens->at(keyIdent->t_identifier);
            m_token = &tk;
            if (m_pos >= tk.begin() && m_pos <= tk.end()) {
                _traverseFinished = true;
                if (keyIdent->isLookable())
                    m_symbol = elementsType->lookupMember(keyIdent, this);
                return false;
            }
        }
    }

    return true;
}

}   // namespace GoTools
