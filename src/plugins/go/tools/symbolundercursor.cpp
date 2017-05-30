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

namespace GoTools {

SymbolUnderCursor::SymbolUnderCursor(GoSource::Ptr doc)
    : ASTVisitor(doc->translationUnit())
    , ExprTypeResolver()
    , m_doc(doc)
    , m_symbol(0)
    , m_token(0)
{
    _tokens = translationUnit()->tokens();
}

TextEditor::TextEditorWidget::Link SymbolUnderCursor::link(unsigned pos)
{
    m_pos = pos;
    defineSymbolUnderCursor();

    if (m_symbol && m_token) {
        TextEditor::TextEditorWidget::Link link;
        link.linkTextStart = m_token->begin();
        link.linkTextEnd = m_token->end();
        m_symbol->fileScope()->fillLink(link, m_symbol->sourceLocation());
        return link;
    }

    return TextEditor::TextEditorWidget::Link();
}

QString SymbolUnderCursor::typeDescription(unsigned pos)
{
    m_pos = pos;
    defineSymbolUnderCursor(DescribeType);
    return m_symbolTypeDescription;
}

void SymbolUnderCursor::defineSymbolUnderCursor(UseReason reason)
{
    m_symbol = 0;
    m_token = 0;
    m_useReason = reason;
    m_symbolTypeDescription.clear();

    if (m_doc->translationUnit() && m_snapshot) {
        m_snapshot->runProtectedTask(
            [this]() -> void {
                if (FileAST *fileAst = m_doc->translationUnit()->fileAst()) {
                    m_currentScope = fileAst->scope;
                    m_currentIndex = fileAst->scope->indexInSnapshot();
                    if (m_currentIndex != -1) {
                        m_ended = false;

                        for (DeclListAST *it = fileAst->decls; it; it = it->next) {
                            const Token &firstToken = _tokens->at(it->value->firstToken());
                            const Token &lastToken = _tokens->at(it->value->lastToken());
                            if (m_pos >= firstToken.begin() && m_pos <= lastToken.end()) {
                                accept(it->value);
                                break;
                            }

                            if (m_pos <= firstToken.begin()) {
                                break;
                            }
                        }

                        if (m_symbol) {
                            switch (m_useReason) {
                                case Link:
                                    break;
                                case DescribeType:
                                    switchScope(m_symbol->owner());
                                    m_symbolTypeDescription = m_symbol->describeType(this);
                                    break;
                            }
                        }

                        eraseResolvedTypes();
                    }
                }
            }
        );
    }
}

bool SymbolUnderCursor::preVisit(AST *)
{ return !m_ended; }

bool SymbolUnderCursor::visit(DeclIdentAST *ast)
{
    const Token &tk = _tokens->at(ast->t_identifier);
    m_token = &tk;

    if (m_pos >= tk.begin() && m_pos <= tk.end()) {
        m_ended = true;
        m_symbol = ast->symbol;
    }

    else if (m_pos <= tk.end())
        m_ended = true;

    return false;
}

bool SymbolUnderCursor::visit(IdentAST *ast)
{
    const Token &tk = _tokens->at(ast->t_identifier);
    m_token = &tk;

    if (m_pos >= tk.begin() && m_pos <= tk.end()) {
        m_ended = true;
        if (ast->isLookable())
            m_symbol = m_currentScope->lookupMember(ast, this);
    }

    else if (m_pos <= tk.end())
        m_ended = true;

    return false;
}

bool SymbolUnderCursor::visit(TypeIdentAST *ast)
{
    const Token &tk = _tokens->at(ast->ident->t_identifier);
    m_token = &tk;

    if (m_pos >= tk.begin() && m_pos <= tk.end()) {
        m_ended = true;
        IdentAST *ident = ast->ident;
        if (ident->isLookable())
            m_symbol = m_currentScope->lookupMember(ident, this);
    }

    else if (m_pos <= tk.end())
        m_ended = true;

    return false;
}

bool SymbolUnderCursor::visit(PackageTypeAST *ast)
{
    const Token &tk = _tokens->at(ast->typeName->t_identifier);
    m_token = &tk;

    if (m_pos >= tk.begin() && m_pos <= tk.end()) {
        m_ended = true;
        if (ast->typeName->isLookable()) {
            QString packageAlias(ast->packageAlias->ident->toString());
            if (PackageType *context = m_snapshot->packageTypeForAlias(m_currentIndex, packageAlias))
                m_symbol = context->lookupMember(ast->typeName, this);
        }
    }

    else if (m_pos <= tk.end())
        m_ended = true;

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
        m_ended = true;
        if (ast->sel->isLookable()) {
            int derefLevel = 0;
            if (Type *type = resolveExpr(ast->x, derefLevel)) {
                derefLevel += type->refLevel();
                if (derefLevel == 0 || derefLevel == -1) {
                    if (Type *baseTyp = type->baseType()) {
                        m_symbol = baseTyp->lookupMember(ast->sel, this);
                    }
                }
            }
        }
    }

    return false;
}

bool SymbolUnderCursor::visit(FuncDeclAST *ast)
{
    accept(ast->recv);
    if (!m_ended) {
        accept(ast->name);
        if (!m_ended) {
            accept(ast->type);
            if (!m_ended) {
                Scope *scope = switchScope(ast->scope);
                accept(ast->body);
                switchScope(scope);
            }
        }
    }

    return false;
}

bool SymbolUnderCursor::visit(BlockStmtAST *ast)
{
    for (StmtListAST *it = ast->list; it; it = it->next) {
        const Token &firstToken = _tokens->at(it->value->firstToken());
        const Token &lastToken = _tokens->at(it->value->lastToken());
        if (m_pos >= firstToken.begin() && m_pos <= lastToken.end()) {
            Scope *scope = switchScope(ast->scope);
            accept(it->value);
            switchScope(scope);
            break;
        }
        if (m_pos <= firstToken.begin()) {
            m_ended = true;
            break;
        }
    }

    return false;
}

bool SymbolUnderCursor::visit(IfStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->init);
    if (!m_ended) {
        accept(ast->cond);
        if (!m_ended) {
            accept(ast->body);
            if (!m_ended)
                accept(ast->elseStmt);
        }
    }
    switchScope(scope);

    return false;
}

bool SymbolUnderCursor::visit(RangeStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->key);
    if (!m_ended) {
        accept(ast->value);
        if (!m_ended) {
            accept(ast->x);
            if (!m_ended)
                accept(ast->body);
        }
    }
    switchScope(scope);

    return false;
}

bool SymbolUnderCursor::visit(ForStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->init);
    if (!m_ended) {
        accept(ast->cond);
        if (!m_ended) {
            accept(ast->post);
            if (!m_ended)
                accept(ast->body);
        }
    }
    switchScope(scope);

    return false;
}

bool SymbolUnderCursor::visit(TypeSwitchStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->init);
    if (!m_ended) {
        accept(ast->assign);
        if (!m_ended)
            accept(ast->body);
    }
    switchScope(scope);

    return false;
}

bool SymbolUnderCursor::visit(SwitchStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->init);
    if (!m_ended) {
        accept(ast->tag);
        if (!m_ended)
            accept(ast->body);
    }
    switchScope(scope);

    return false;
}

bool SymbolUnderCursor::visit(CaseClauseAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->list);
    if (!m_ended)
        accept(ast->body);
    switchScope(scope);

    return false;
}

bool SymbolUnderCursor::visit(CompositeLitAST *ast)
{
    if (ast->elements) {
        const Token &firstToken = _tokens->at(ast->elements->firstToken());
        const Token &lastToken = _tokens->at(ast->elements->lastToken());
        if (m_pos >= firstToken.begin() && m_pos <= lastToken.end()) {
            Type *type = 0;
            if (ExprAST *typeExpr = ast->type) {
                type = typeExpr->asType();
                if (!type) {
                    type = resolveCompositExpr(ast);
                }
            } else if (!m_nestedCimpositLitType.empty()) {
                type = m_nestedCimpositLitType.top();
                if (type)
                    type = type->elementsType(this);
            }

            m_nestedCimpositLitType.push(type);
            accept(ast->elements);
            m_nestedCimpositLitType.pop();
            return false;
        }
    }

    return true;
}

bool SymbolUnderCursor::visit(KeyValueExprAST *ast)
{
    Type *elementsType = m_nestedCimpositLitType.empty() ? 0 : m_nestedCimpositLitType.top();
    if (elementsType && ast->key) {
        if (IdentAST *keyIdent = ast->key->asIdent()) {
            const Token &tk = _tokens->at(keyIdent->t_identifier);
            m_token = &tk;
            if (m_pos >= tk.begin() && m_pos <= tk.end()) {
                m_ended = true;
                if (keyIdent->isLookable())
                    m_symbol = elementsType->lookupMember(keyIdent, this);
                return false;
            }
        }
    }

    return true;
}

}   // namespace GoTools
