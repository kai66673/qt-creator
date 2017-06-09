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
#include "scopevisitor.h"
#include "ast.h"

namespace GoTools {

ScopeSwitchVisitor::ScopeSwitchVisitor(GoSource::Ptr doc)
    : ASTVisitor(doc->translationUnit())
    , ExprTypeResolver()
{ }

bool ScopeSwitchVisitor::visit(FuncDeclAST *ast)
{
    accept(ast->recv);
    accept(ast->name);
    accept(ast->type);

    Scope *scope = switchScope(ast->scope);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(BlockStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->list);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(IfStmtAST *ast)
{
    accept(ast->init);

    Scope *scope = switchScope(ast->scope);
    accept(ast->cond);
    accept(ast->body);
    accept(ast->elseStmt);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(RangeStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->key);
    accept(ast->value);
    accept(ast->x);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(ForStmtAST *ast)
{
    accept(ast->init);

    Scope *scope = switchScope(ast->scope);
    accept(ast->cond);
    accept(ast->post);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(TypeSwitchStmtAST *ast)
{
    accept(ast->init);

    Scope *scope = switchScope(ast->scope);
    accept(ast->assign);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(SwitchStmtAST *ast)
{
    accept(ast->init);

    Scope *scope = switchScope(ast->scope);
    accept(ast->tag);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(CaseClauseAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->list);
    accept(ast->body);
    switchScope(scope);

    return false;
}

ScopePositionVisitor::ScopePositionVisitor(GoSource::Ptr doc)
    : ASTVisitor(doc->translationUnit())
    , ExprTypeResolver()
{ }

bool ScopePositionVisitor::preVisit(AST *)
{ return !m_ended; }

bool ScopePositionVisitor::visit(FuncDeclAST *ast)
{
    if (m_pos < _tokens->at(ast->firstToken()).begin()) {
        m_ended = true;
        return false;
    }

    if (m_pos <= _tokens->at(ast->lastToken()).end()) {
        accept(ast->recv);
        accept(ast->name);
        accept(ast->type);
        if (!m_ended) {
            switchScope(ast->scope);
            accept(ast->body);
            m_ended = true;
        }
    }

    return false;
}

bool ScopePositionVisitor::visit(BlockStmtAST *ast)
{
    switchScope(ast->scope);
    for (StmtListAST *it = ast->list; it; it = it->next) {
        if (m_pos < _tokens->at(it->value->firstToken()).begin()) {
            m_ended = true;
            break;
        }

        if (m_pos <= _tokens->at(it->value->lastToken()).end()) {
            accept(it->value);
            m_ended = true;
            break;
        }
    }

    return false;
}

bool ScopePositionVisitor::visit(IfStmtAST *ast)
{
    if (m_pos < _tokens->at(ast->firstToken()).begin()) {
        m_ended = true;
        return false;
    }

    if (m_pos <= _tokens->at(ast->lastToken()).end()) {
        accept(ast->init);
        if (!m_ended) {
            switchScope(ast->scope);
            accept(ast->cond);
            accept(ast->body);
            accept(ast->elseStmt);
            m_ended = true;
        }
    }

    return false;
}

bool ScopePositionVisitor::visit(RangeStmtAST *ast)
{
    if (m_pos < _tokens->at(ast->firstToken()).begin()) {
        m_ended = true;
        return false;
    }

    if (m_pos <= _tokens->at(ast->lastToken()).end()) {
        accept(ast->key);
        accept(ast->value);
        accept(ast->x);
        if (!m_ended) {
            switchScope(ast->scope);
            accept(ast->body);
            m_ended = true;
        }
    }

    return false;
}

bool ScopePositionVisitor::visit(ForStmtAST *ast)
{
    if (m_pos < _tokens->at(ast->firstToken()).begin()) {
        m_ended = true;
        return false;
    }

    if (m_pos <= _tokens->at(ast->lastToken()).end()) {
        accept(ast->init);
        if (!m_ended) {
            switchScope(ast->scope);
            accept(ast->cond);
            accept(ast->post);
            accept(ast->body);
            m_ended = true;
        }
    }

    return false;
}

bool ScopePositionVisitor::visit(TypeSwitchStmtAST *ast)
{
    if (m_pos < _tokens->at(ast->firstToken()).begin()) {
        m_ended = true;
        return false;
    }

    if (m_pos <= _tokens->at(ast->lastToken()).end()) {
        accept(ast->init);
        if (!m_ended) {
            switchScope(ast->scope);
            accept(ast->assign);
            accept(ast->body);
            m_ended = true;
        }
    }

    return false;
}

bool ScopePositionVisitor::visit(SwitchStmtAST *ast)
{
    if (m_pos < _tokens->at(ast->firstToken()).begin()) {
        m_ended = true;
        return false;
    }

    if (m_pos <= _tokens->at(ast->lastToken()).end()) {
        accept(ast->init);
        accept(ast->tag);
        if (!m_ended) {
            switchScope(ast->scope);
            accept(ast->body);
            m_ended = true;
        }
    }

    return false;
}

bool ScopePositionVisitor::visit(CaseClauseAST *ast)
{
    if (m_pos < _tokens->at(ast->firstToken()).begin()) {
        m_ended = true;
        return false;
    }

    if (m_pos <= _tokens->at(ast->lastToken()).end()) {
        accept(ast->list);
        if (!m_ended) {
            switchScope(ast->scope);
            accept(ast->body);
            m_ended = true;
        }
    }

    return false;
}

}   // namespace GoTools
